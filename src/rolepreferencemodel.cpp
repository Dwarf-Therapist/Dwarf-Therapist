/*
Dwarf Therapist
Copyright (c) 2018 Clément Vuchener

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include "rolepreferencemodel.h"

#include <QProgressDialog>
#include <QtConcurrent>

#include "dfinstance.h"
#include "rolepreference.h"
#include "preference.h"
#include "material.h"
#include "plant.h"
#include "races.h"
#include "item.h"
#include "itemweaponsubtype.h"

RolePreferenceModel::node_t::node_t(std::unique_ptr<GenericRolePreference> &&pref, node_t *parent)
    : parent(parent)
    , pref(std::move(pref))
{
}

RolePreferenceModel::node_t::~node_t()
{
}

template<typename T, typename... Args>
RolePreferenceModel::node_t *RolePreferenceModel::node_t::add_category(Args &&... args)
{
    sub_categories.emplace_back(std::make_unique<node_t>(
        std::make_unique<T>(std::forward<Args>(args)...),
        this
    ));
    auto child = sub_categories.back().get();
    child->row = sub_categories.size()-1;
    return child;
}

template<typename T, typename... Args>
RolePreferenceModel::node_t *RolePreferenceModel::add_top_category(Args &&... args)
{
    m_prefs.emplace_back(std::make_unique<node_t>(std::make_unique<T>(std::forward<Args>(args)...)));
    auto child = m_prefs.back().get();
    child->row = m_prefs.size()-1;
    return child;
}

// Lists for filtering possible material preferences
static const std::vector<std::pair<MATERIAL_FLAGS, MATERIAL_STATES>> MaterialsByFlag = {
    { IS_STONE, SOLID }, // sub-category: CRYSTAL_GLASSABLE?
    { IS_METAL, SOLID },
    { IS_GEM, SOLID },
    { IS_WOOD, SOLID },
    { IS_GLASS, SOLID },
    { LEATHER, SOLID },
    { HORN, SOLID },
    { PEARL, SOLID },
    { TOOTH, SOLID },
    { ITEMS_DELICATE, SOLID },
    { BONE, SOLID },
    { SHELL, SOLID },
    { SILK, SOLID },
    { YARN, SOLID },
    { THREAD_PLANT, SOLID },
    // Dyes?
};
static const std::vector<std::pair<const char *, MATERIAL_STATES>> MaterialsByReaction = {
    { "PARCHMENT", PRESSED },
    { "PAPER_PLANT", PRESSED },
    { "PAPER_SLURRY", PRESSED },
};

static const std::set<ITEM_TYPE> ItemTypes = {
    // Equipment
    WEAPON, AMMO,
    ARMOR, SHOES, SHIELD, HELM, GLOVES, PANTS, BACKPACK, QUIVER,
    // Furniture
    DOOR, FLOODGATE, BED, CHAIR, WINDOW, CAGE, BARREL, TABLE, COFFIN, STATUE,
    BOX, BIN, ARMORSTAND, WEAPONRACK, CABINET, HATCH_COVER, GRATE, QUERN,
    MILLSTONE, TRACTION_BENCH, SLAB,
    // Craft
    FIGURINE, AMULET, SCEPTER, CROWN, RING, EARRING, BRACELET, GEM,
    // Misc
    CHAIN, FLASK, GOBLET, INSTRUMENT, TOY, BUCKET, ANIMALTRAP, ANVIL, TOTEM,
    CATAPULTPARTS, BALLISTAPARTS, SIEGEAMMO, TRAPCOMP, COIN, SPLINT, CRUTCH,
    TOOL,
};

RolePreferenceModel::RolePreferenceModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    auto materials = add_top_category<GenericRolePreference>(LIKE_MATERIAL, tr("Materials"));
    //any material types that we want to add to the general category section go here
    for (const auto &p: MaterialsByFlag) {
        auto flag = p.first;
        auto state = p.second;
        auto cat = materials->add_category<GenericMaterialRolePreference>(
                Material::get_material_flag_desc(flag, state),
                state, flag);
        if (flag == IS_STONE) {
            cat->add_category<GenericMaterialRolePreference>(
                tr("Glazes & Stoneware"), SOLID,
                ITEMS_QUERN, NO_STONE_STOCKPILE);
        }
    }
    for (const auto t: {std::make_tuple(tr("Parchments"), PRESSED, "PARCHMENT"),
                        std::make_tuple(tr("Paper plants"), PRESSED, "PAPER_PLANT"),
                        std::make_tuple(tr("Papers"), PRESSED, "PAPER_SLURRY")}) {
        auto title = std::get<0>(t);
        auto state = std::get<1>(t);
        auto reaction = std::get<2>(t);
        materials->add_category<MaterialReactionRolePreference>(title, state, reaction);
    }

    auto plants = add_top_category<GenericRolePreference>(LIKE_PLANT, tr("Plants"));
    //general category for plants used for alcohol
    plants->add_category<GenericRolePreference>(LIKE_PLANT, tr("Plants (Alcohol)"), P_DRINK);
    //general category for crops plant or gather
    plants->add_category<GenericRolePreference>(LIKE_PLANT, tr("Plants (Crops)"), P_CROP)
        ->add_category<GenericRolePreference>(LIKE_PLANT, tr("Plants (Crops Plantable)"), P_CROP, P_SEED);
    //general category for millable plants
    plants->add_category<GenericRolePreference>(LIKE_PLANT, tr("Plants (Millable)"), P_MILL);
    //general category for plants used for processing/threshing
    plants->add_category<GenericRolePreference>(LIKE_PLANT, tr("Plants (Extracts)"), P_HAS_EXTRACTS);

    auto items = add_top_category<GenericRolePreference>(LIKE_ITEM, tr("Items"));
    // general item preferences
    auto clothing = items->add_category<GenericRolePreference>(LIKE_ITEM, tr("Clothing (Any)"), ITEM_IS_CLOTHING);
    auto armor = items->add_category<GenericRolePreference>(LIKE_ITEM, tr("Armor (Any)"), ITEM_IS_ARMOR);
    auto trade_good = items->add_category<GenericRolePreference>(LIKE_ITEM, tr("Trade Goods"), ITEM_TYPE_IS_TRADE_GOOD);
    for (ITEM_TYPE itype: ItemTypes) {
        if (itype == WEAPON) {
            items->add_category<GenericItemRolePreference>(tr("Weapons (Ranged)"), WEAPON, ITEM_RANGED_WEAPON);
            items->add_category<GenericItemRolePreference>(tr("Weapons (Melee)"), WEAPON, ITEM_MELEE_WEAPON);
        }
        else {
            QString name = Item::get_item_name_plural(itype);

            //add all item types as a group to the general categories
            if(Item::is_trade_good(itype)){
                trade_good->add_category<GenericItemRolePreference>(name, itype);
            }
            else if (Item::is_armor_type(itype,false)) {
                armor->add_category<GenericItemRolePreference>(name, itype, ITEM_IS_ARMOR);
                clothing->add_category<GenericItemRolePreference>(
                        Item::get_item_clothing_name(itype), itype, ITEM_IS_CLOTHING);
            }
            else {
                items->add_category<GenericItemRolePreference>(name, itype);
            }
        }
    }

    auto creatures = add_top_category<GenericRolePreference>(LIKE_CREATURE, tr("Creatures"));
    //add general categories for groups of creatures
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Creatures (Hateable)"),
            HATEABLE);
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Vermin fish"),
            VERMIN_FISH);
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Creatures (Trainable)"),
            TRAINABLE);
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Creatures (Milkable)"),
            MILKABLE);
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Creatures (Fishable)"),
            FISHABLE);
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Creatures (Shearable)"),
            SHEARABLE);
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Creatures (Extracts)"),
            HAS_EXTRACTS)
        ->add_category<GenericRolePreference>(
                LIKE_CREATURE, tr("Creatures (Fish Extracts)"),
                HAS_EXTRACTS, FISHABLE);
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Creatures (Butcherable)"),
            BUTCHERABLE);
    creatures->add_category<GenericRolePreference>(
            LIKE_CREATURE, tr("Creatures (Domestic)"),
            DOMESTIC);

    add_top_category<GenericRolePreference>(LIKE_TREE, tr("Trees"));
    add_top_category<GenericRolePreference>(LIKE_COLOR, tr("Colors"));
    add_top_category<GenericRolePreference>(LIKE_SHAPE, tr("Shapes"));
    add_top_category<GenericRolePreference>(LIKE_OUTDOORS, tr("Outdoors"), 999);
}

RolePreferenceModel::~RolePreferenceModel()
{
}

void RolePreferenceModel::set_df_instance(DFInstance *df)
{
    beginResetModel();
    clear_exact_prefs(m_prefs);
    m_loaded_raws = false;
    m_df = df;
    endResetModel();
}

void RolePreferenceModel::add_material(Material *m)
{
    std::set<MATERIAL_STATES> states;
    for (const auto &p: MaterialsByFlag) {
        auto flag = p.first;
        auto state = p.second;
        if (m->flags().has_flag(flag))
            states.insert(state);
    }
    for (const auto &p: MaterialsByReaction) {
        auto reaction = p.first;
        auto state = p.second;
        if (m->has_reaction(reaction))
            states.insert(state);
    }
    for (auto state: states) {
        add_exact_pref(m_prefs,
                       std::make_shared<ExactMaterialRolePreference>(m, state),
                       MaterialPreference(m, state));
    }
}

bool RolePreferenceModel::add_exact_pref(std::vector<std::unique_ptr<node_t>> &categories,
                                         const std::shared_ptr<ExactRolePreference> &role_pref,
                                         const Preference &pref)
{
    bool matched = false;
    for (auto &category: categories) {
        if (category->pref->match(&pref, nullptr)) {
            if (!add_exact_pref(category->sub_categories, role_pref, pref))
                category->exact_prefs.push_back(role_pref);
            matched = true;
            ++category->exact_pref_count;
        }
    }
    return matched;
}

void RolePreferenceModel::clear_exact_prefs(std::vector<std::unique_ptr<node_t>> &categories)
{
    for (auto &category: categories) {
        clear_exact_prefs(category->sub_categories);
        category->exact_prefs.clear();
    }
}

void RolePreferenceModel::load_pref_from_raws(QWidget *parent)
{
    if (!m_df || m_loaded_raws)
        return;

    QProgressDialog progress(tr("Reading preferences from raws..."), QString(), 0, 0, parent);
    QFutureWatcher<void> watcher;
    connect(&watcher, SIGNAL(finished()), &progress, SLOT(reset()));

    auto future = QtConcurrent::run([this] () {
        beginResetModel();
        clear_exact_prefs(m_prefs);

        // non-plant and non-creature materials
        for (Material *m: m_df->get_inorganic_materials())
            add_material(m);
        for (Material *m: m_df->get_base_materials())
            add_material(m);

        // Plants (and plant materials)
        foreach(Plant *p, m_df->get_plants()){
            auto plant_pref = std::make_shared<ExactRolePreference>(p);
            if(p->flags().has_flag(P_SAPLING) || p->flags().has_flag(P_TREE))
                add_exact_pref(m_prefs, plant_pref, TreePreference(p));
            else
                add_exact_pref(m_prefs, plant_pref, PlantPreference(p));

            for (Material *m: p->get_plant_materials())
                add_material(m);
        }

        // Creatures (and creature materials)
        foreach(Race *r, m_df->get_races()){
            if(r->flags().has_flag(WAGON))
                continue;

            if  (!r->pref_strings().isEmpty() || r->caste_flag(DOMESTIC)) {
                add_exact_pref(m_prefs,
                               std::make_shared<ExactRolePreference>(r),
                               CreaturePreference(r));
            }

            for (Material *m: r->get_creature_materials().values())
                add_material(m);
        }

        // Items
        auto item_list = m_df->get_all_item_defs();
        for (ITEM_TYPE itype: ItemTypes) {
            if (Item::has_subtypes(itype)) {
                for(int sub_id = 0; sub_id < item_list.value(itype).count(); sub_id++){
                    ItemSubtype *subtype = m_df->get_item_subtype(itype, sub_id);
                    if (subtype->flags().has_flag(ITEM_GENERATED))
                        continue;
                    add_exact_pref(m_prefs,
                                   std::make_shared<ExactItemRolePreference>(subtype),
                                   ItemPreference(subtype));
                }
            }
            else {
                add_exact_pref(m_prefs,
                               std::make_shared<ExactItemRolePreference>(Item::get_item_name_plural(itype), itype),
                               ItemPreference(itype));
            }
        }

        // Colors
        auto color_vector = m_df->get_colors();
        for (int i = 0; i < color_vector.size(); ++i) {
            auto name = m_df->get_preference_other_name(i, LIKE_COLOR);
            add_exact_pref(m_prefs,
                           std::make_shared<ExactRolePreference>(LIKE_COLOR, name),
                           Preference(LIKE_COLOR, name));
        }

        // Shapes
        auto shape_vector = m_df->get_shapes();
        for (auto shape: shape_vector) {
            auto name = m_df->get_shape_name(shape);
            add_exact_pref(m_prefs,
                           std::make_shared<ExactRolePreference>(LIKE_SHAPE, name),
                           Preference(LIKE_SHAPE, name));
        }

        m_loaded_raws = true;
        endResetModel();
    });// end concurrent run
    watcher.setFuture(future);

    progress.exec();
}

// In QModelIndex created by this model, the internal pointer is the pointer
// to the *parent* node_t or nullptr for top level categories.

// casting to non-const void pointer for storing const pointer inside QModelIndex
template<typename T>
static inline void *void_cast(const T *ptr) { return static_cast<void *>(const_cast<T *>(ptr)); }

QModelIndex RolePreferenceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        auto grandparent_node = static_cast<const node_t *>(parent.internalPointer());
        auto parent_node = grandparent_node
                ? grandparent_node->sub_categories[parent.row()].get()
                : m_prefs[parent.row()].get();
        return createIndex(row, column, void_cast(parent_node));
    }
    else { // root
        return createIndex(row, column, nullptr);
    }
}

QModelIndex RolePreferenceModel::parent(const QModelIndex &index) const
{
    auto parent_node = static_cast<const node_t *>(index.internalPointer());
    if (parent_node)
        return createIndex(parent_node->row, 0, void_cast(parent_node->parent));
    else
        return QModelIndex();
}

QModelIndex RolePreferenceModel::sibling(int row, int column, const QModelIndex &index) const
{
    return createIndex(row, column, index.internalPointer());
}

int RolePreferenceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        auto grandparent_node = static_cast<const node_t *>(parent.internalPointer());
        auto &parent_categories = grandparent_node
                ? grandparent_node->sub_categories
                : m_prefs;
        if (parent.row() >= (int)parent_categories.size()) // exact preference
            return 0;

        auto &parent_node = parent_categories[parent.row()];
        return parent_node->sub_categories.size() + parent_node->exact_prefs.size();
    }
    else { // root item
        return m_prefs.size();
    }
}

int RolePreferenceModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant RolePreferenceModel::data(const QModelIndex &index, int role) const
{
    struct display_t {
        QVariant operator() (const node_t *cat) const {
            return cat->pref->get_name();
        }
        QVariant operator() (const ExactRolePreference *pref) const {
            return pref->get_name();
        }
    };
    struct tooltip_t {
        QVariant operator() (const node_t *cat) const {
            return tr("Generic %1 preference for any %2")
                .arg(Preference::get_pref_desc(cat->pref->get_pref_category()).toLower())
                .arg(cat->pref->get_name().toLower());
        }
        QVariant operator() (const ExactRolePreference *pref) const {
            return tr("Exact preference for %1").arg(pref->get_name().toLower());
        }
    };
    struct sort_t {
        QVariant operator() (const node_t *cat) const {
            return QString("1%1").arg(cat->pref->get_name());
        }
        QVariant operator() (const ExactRolePreference *pref) const {
            return QString("2%1").arg(pref->get_name());
        }
    };

    switch (role) {
    case Qt::DisplayRole:
        return apply_to_item<QVariant>(display_t(), index);
    case Qt::ToolTipRole:
        return apply_to_item<QVariant>(tooltip_t(), index);
    case SortRole:
        return apply_to_item<QVariant>(sort_t(), index);
    default:
        return QVariant();
    }
}

QVariant RolePreferenceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal || section != 0)
        return QVariant();

    return tr("Preference");
}

const RolePreference *RolePreferenceModel::getPreference(const QModelIndex &index) const
{
    struct get_pref_t {
        const RolePreference *operator() (const node_t *cat) const {
            return cat->pref.get();
        }
        const RolePreference *operator() (const ExactRolePreference *pref) const {
            return pref;
        }
    };
    return apply_to_item<const RolePreference *>(get_pref_t(), index);
}

template<typename Ret, typename Function>
Ret RolePreferenceModel::apply_to_item(Function function, const QModelIndex &index) const
{
    if (!index.isValid())
        return Ret();
    auto parent_node = static_cast<const node_t *>(index.internalPointer());
    if (parent_node) {
        int ncat = parent_node->sub_categories.size();
        if (index.row() < ncat)
            return function(parent_node->sub_categories[index.row()].get());
        else
            return function(parent_node->exact_prefs[index.row()-ncat].get());
    }
    else {
        return function(m_prefs[index.row()].get());
    }
}
