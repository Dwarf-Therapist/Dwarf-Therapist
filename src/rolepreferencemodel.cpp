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
#include "material.h"
#include "plant.h"
#include "races.h"
#include "item.h"
#include "itemweaponsubtype.h"

QString RolePreferenceModel::get_category_name(GenericCategory category)
{
    switch (category) {
    case ITEM:
        return tr("General Items");
    case EQUIPMENT:
        return tr("General Equipment");
    case MATERIAL:
        return tr("General Materials");
    case CREATURE:
        return tr("General Creatures");
    case TRADE_GOOD:
        return tr("General Trade Goods");
    case PLANT_TREE:
        return tr("General Plants & Trees");
    case OTHER:
        return tr("General Other");
    default:
        return tr("Missing category name");
    }
}

QString RolePreferenceModel::get_category_name(ExactCategory category)
{
    switch (category) {
    case GEMS:
        return RolePreferenceModel::tr("Gems");
    case GLASS:
        return RolePreferenceModel::tr("Glass & Crystals");
    case METALS:
        return RolePreferenceModel::tr("Metals");
    case STONE:
        return RolePreferenceModel::tr("Stone & Ores");
    case WOOD:
        return RolePreferenceModel::tr("Wood");
    case GLAZES_WARES:
        return RolePreferenceModel::tr("Glazes & Stoneware");
    case FABRICS:
        return RolePreferenceModel::tr("Fabrics & Dyes");
    case PAPERS:
        return RolePreferenceModel::tr("Papers");
    case LEATHERS:
        return RolePreferenceModel::tr("Leathers");
    case PARCHMENTS:
        return RolePreferenceModel::tr("Parchments");
    case OTHER_MATERIALS:
        return RolePreferenceModel::tr("Other materials");
    case PLANTS:
        return RolePreferenceModel::tr("Plants");
    case PLANTS_ALCOHOL:
        return RolePreferenceModel::tr("Plants (Alcohol)");
    case PLANTS_CROPS:
        return RolePreferenceModel::tr("Plants (Crops)");
    case PLANTS_CROPS_PLANTABLE:
        return RolePreferenceModel::tr("Plants (Crops Plantable)");
    case PLANTS_MILL:
        return RolePreferenceModel::tr("Plants (Mill)");
    case PLANTS_EXTRACT:
        return RolePreferenceModel::tr("Plants (Extracts)");
    case TREES:
        return RolePreferenceModel::tr("Trees");
    case CREATURES:
        return RolePreferenceModel::tr("Creatures (Other)");
    case CREATURES_HATEABLE:
        return RolePreferenceModel::tr("Creatures (Hateable)");
    case CREATURES_TRAINABLE:
        return RolePreferenceModel::tr("Creatures (Trainable)");
    case CREATURES_MILKABLE:
        return RolePreferenceModel::tr("Creatures (Milkable)");
    case CREATURES_EXTRACTS:
        return RolePreferenceModel::tr("Creatures (Extracts)");
    case CREATURES_EXTRACTS_FISH:
        return RolePreferenceModel::tr("Creatures (Fish Extracts)");
    case CREATURES_FISHABLE:
        return RolePreferenceModel::tr("Creatures (Fishable)");
    case CREATURES_SHEARABLE:
        return RolePreferenceModel::tr("Creatures (Shearable)");
    case CREATURES_BUTCHER:
        return RolePreferenceModel::tr("Creatures (Butcherable)");
    case CREATURES_DOMESTIC:
        return RolePreferenceModel::tr("Creatures (Domestic)");
    case WEAPONS_MELEE:
        return RolePreferenceModel::tr("Weapons (Melee)");
    case WEAPONS_RANGED:
        return RolePreferenceModel::tr("Weapons (Ranged)");
    default:
        return RolePreferenceModel::tr("Missing category name");
    }
}

//setup a list of item exclusions. these are item types that are not found in item preferences
//weapons are also ignored because we'll handle them manually to split them into ranged and melee categories
static const std::set<ITEM_TYPE> item_ignore = {
    BAR, SMALLGEM, BLOCKS, ROUGH, BOULDER, WOOD, CORPSE, CORPSEPIECE, REMAINS,
    FISH_RAW, VERMIN, IS_PET, SKIN_TANNED, THREAD, CLOTH, BALLISTAARROWHEAD,
    TRAPPARTS, FOOD, GLOB, ROCK, PIPE_SECTION, ORTHOPEDIC_CAST, EGG, BOOK,
    SHEET, WEAPON,
//additionally ignore food types, since they can only be a preference as a consumable
    MEAT, FISH, CHEESE, PLANT, DRINK, POWDER_MISC, LEAVES_FRUIT, LIQUID_MISC, SEEDS,
};

RolePreferenceModel::RolePreferenceModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    //setup general categories
    auto &general_item = m_general_prefs[ITEM];
    auto &general_equip = m_general_prefs[EQUIPMENT];
    auto &general_material = m_general_prefs[MATERIAL];
    auto &general_creature = m_general_prefs[CREATURE];
    auto &general_trade_good = m_general_prefs[TRADE_GOOD];
    auto &general_plant_tree = m_general_prefs[PLANT_TREE];
    auto &general_other = m_general_prefs[OTHER];

    //also add trees to general category. don't need a flag as trees is a pref category
    auto p_trees = std::make_unique<RolePreference>(LIKE_TREE, tr("Trees"));
    //p_trees->add_flag(77); //is tree flag
    general_plant_tree.emplace_back(std::move(p_trees));

    //any material types that we want to add to the general category section go here
    for (const auto t: {std::make_tuple(BONE, ANY_STATE),
                        std::make_tuple(TOOTH, ANY_STATE),
                        std::make_tuple(HORN, ANY_STATE),
                        std::make_tuple(PEARL, ANY_STATE),
                        std::make_tuple(SHELL, ANY_STATE),
                        std::make_tuple(LEATHER, ANY_STATE),
                        std::make_tuple(SILK, ANY_STATE),
                        std::make_tuple(IS_GLASS, ANY_STATE),
                        std::make_tuple(IS_WOOD, ANY_STATE),
                        std::make_tuple(THREAD_PLANT, SOLID), // fabric
                        std::make_tuple(THREAD_PLANT, PRESSED), // paper
                        std::make_tuple(YARN, ANY_STATE)}){
        auto flag = std::get<0>(t);
        auto state = std::get<1>(t);
        general_material.emplace_back(std::make_unique<GenericMaterialRolePreference>(
                Material::get_material_flag_desc(flag, state),
                state, flag));
    }
    for (const auto t: {std::make_tuple(tr("Parchments"), PRESSED, "PARCHMENT"),
                        std::make_tuple(tr("Paper plants"), PRESSED, "PAPER_PLANT")}) {
        auto title = std::get<0>(t);
        auto state = std::get<1>(t);
        auto reaction = std::get<2>(t);
        general_material.emplace_back(std::make_unique<MaterialReactionRolePreference>(
                title, state, reaction));
    }

    //general category for plants used for alcohol
    general_plant_tree.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_PLANT, tr("Plants (Alcohol)"), P_DRINK));
    //general category for crops plant or gather
    general_plant_tree.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_PLANT, tr("Plants (Crops)"), P_CROP));
    //general category for plantable crops
    general_plant_tree.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_PLANT, tr("Plants (Crops Plantable)"), P_CROP, P_SEED));
    //general category for millable plants
    general_plant_tree.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_PLANT, tr("Plants (Millable)"), P_MILL));
    //general category for plants used for processing/threshing
    general_plant_tree.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_PLANT, tr("Plants (Extracts)"), P_HAS_EXTRACTS));

    //special custom preference for outdoors
    general_other.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_OUTDOORS, tr("Outdoors"), 999));

    // general item preferences
    general_equip.emplace_back(std::make_unique<GenericRolePreference>(LIKE_ITEM, tr("Clothing (Any)"), IS_CLOTHING));
    general_equip.emplace_back(std::make_unique<GenericRolePreference>(LIKE_ITEM, tr("Armor (Any)"), IS_ARMOR));
    general_item.emplace_back(std::make_unique<GenericRolePreference>(LIKE_ITEM, tr("Trade Goods"), IS_TRADE_GOOD));

    for(int idx=0; idx < NUM_OF_ITEM_TYPES; idx++){
        ITEM_TYPE itype = static_cast<ITEM_TYPE>(idx);

        if(item_ignore.find(itype) == item_ignore.end()){
            QString name = Item::get_item_name_plural(itype);

            bool is_armor_type = Item::is_armor_type(itype,false);

            //add all item types as a group to the general categories
            if(Item::is_trade_good(itype)){
                general_trade_good.emplace_back(std::make_unique<GenericItemRolePreference>(
                        name, itype, IS_TRADE_GOOD));
            }
            else {
                auto p = std::make_unique<GenericItemRolePreference>(name, itype);
                if (is_armor_type || Item::is_supplies(itype) ||
                        Item::is_melee_equipment(itype) || Item::is_ranged_equipment(itype))
                    general_equip.emplace_back(std::move(p));
                else
                    general_item.emplace_back(std::move(p));
            }
            if(is_armor_type){
                general_equip.emplace_back(std::make_unique<GenericItemRolePreference>(
                        Item::get_item_clothing_name(itype), itype, IS_CLOTHING));
            }
        }
    }

    //add general categories for groups of creatures
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_HATEABLE),
            HATEABLE));
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_EXTRACTS_FISH),
            VERMIN_FISH));
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_TRAINABLE),
            TRAINABLE_HUNTING, TRAINABLE_WAR));
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_MILKABLE),
            MILKABLE));
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_FISHABLE),
            FISHABLE));
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_SHEARABLE),
            SHEARABLE));
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_EXTRACTS),
            HAS_EXTRACTS));
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_BUTCHER),
            BUTCHERABLE));
    general_creature.emplace_back(std::make_unique<GenericRolePreference>(
            LIKE_CREATURE, get_category_name(CREATURES_DOMESTIC),
            DOMESTIC));

    // general weapon preferences
    general_equip.emplace_back(std::make_unique<GenericItemRolePreference>(
            get_category_name(WEAPONS_RANGED),
            WEAPON, ITEMS_WEAPON_RANGED));
    general_equip.emplace_back(std::make_unique<GenericItemRolePreference>(
            get_category_name(WEAPONS_MELEE),
            WEAPON, ITEMS_WEAPON));
}

RolePreferenceModel::~RolePreferenceModel()
{
}

void RolePreferenceModel::set_df_instance(DFInstance *df)
{
    beginResetModel();
    for (auto &cat: m_raw_prefs)
        cat.clear();
    m_item_prefs.clear();
    m_loaded_raws = false;
    m_df = df;
    endResetModel();
}

void RolePreferenceModel::load_material_prefs(QVector<Material*> mats)
{
    foreach(Material *m, mats){
        if(m->is_generated())
            continue;

        //check specific flags
        if(m->flags().has_flag(THREAD_PLANT)) {
            m_raw_prefs[FABRICS].emplace_back(std::make_shared<ExactMaterialRolePreference>(m, SOLID));
            m_raw_prefs[PAPERS].emplace_back(std::make_shared<ExactMaterialRolePreference>(m, PRESSED));
        }
        else if(m->flags().has_flag(IS_DYE))
            m_raw_prefs[FABRICS].emplace_back(std::make_shared<ExactMaterialRolePreference>(m, POWDER));
        else {
            auto p = std::make_shared<ExactMaterialRolePreference>(m, SOLID);
            if (m->flags().has_flag(IS_GEM))
                m_raw_prefs[GEMS].emplace_back(p);
            else if (m->flags().has_flag(IS_GLASS) || m->flags().has_flag(CRYSTAL_GLASSABLE))
                m_raw_prefs[GLASS].emplace_back(p);
            else if (m->flags().has_flag(IS_METAL))
                m_raw_prefs[METALS].emplace_back(p);
            else if(m->flags().has_flag(IS_WOOD))
                m_raw_prefs[WOOD].emplace_back(p);
            else if(m->flags().has_flag(IS_STONE)) {
                if (m->flags().has_flag(ITEMS_QUERN) && m->flags().has_flag(NO_STONE_STOCKPILE))
                    m_raw_prefs[GLAZES_WARES].emplace_back(p);
                else
                    m_raw_prefs[STONE].emplace_back(p);
            }
            else if(m->flags().has_flag(ITEMS_DELICATE))
                m_raw_prefs[OTHER_MATERIALS].emplace_back(p); //check for coral and amber
        }

        //check reactions
        if (m->has_reaction("PAPER_PLANT")) {
            auto p = std::make_shared<ExactMaterialRolePreference>(m, PRESSED);
            m_raw_prefs[PAPERS].emplace_back(p);
        }
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
        for (auto &cat: m_raw_prefs)
            cat.clear();
        m_item_prefs.clear();

        // non-plant and non-creature materials
        load_material_prefs(m_df->get_inorganic_materials());
        load_material_prefs(m_df->get_base_materials());

        // Plants (and plant materials)
        foreach(Plant *p, m_df->get_plants()){
            auto plant_pref = std::make_shared<ExactRolePreference>(p);

            if(p->flags().has_flag(P_SAPLING) || p->flags().has_flag(P_TREE)){
                m_raw_prefs[TREES].emplace_back(plant_pref);
            }else{
                m_raw_prefs[PLANTS].emplace_back(plant_pref);

                if(p->flags().has_flag(P_DRINK)){
                    m_raw_prefs[PLANTS_ALCOHOL].emplace_back(plant_pref);
                }
                if(p->flags().has_flag(P_CROP)){
                    m_raw_prefs[PLANTS_CROPS].emplace_back(plant_pref);
                    if(p->flags().has_flag(P_SEED)){
                        m_raw_prefs[PLANTS_CROPS_PLANTABLE].emplace_back(plant_pref);
                    }
                }
            }

            if(p->flags().has_flag(P_MILL)){
                m_raw_prefs[PLANTS_MILL].emplace_back(plant_pref);
            }
            if(p->flags().has_flag(P_HAS_EXTRACTS)){
                m_raw_prefs[PLANTS_EXTRACT].emplace_back(plant_pref);
            }

            load_material_prefs(p->get_plant_materials());
        }

        // Creatures (and creature materials)
        foreach(Race *r, m_df->get_races()){
            if(r->flags().has_flag(WAGON))
                continue;

            auto p = std::make_shared<ExactRolePreference>(r);

            if(r->caste_flag(DOMESTIC)){
                m_raw_prefs[CREATURES_DOMESTIC].emplace_back(p);
            }

            if(r->flags().has_flag(HATEABLE)){
                m_raw_prefs[CREATURES_HATEABLE].emplace_back(p);
            }else{
                m_raw_prefs[CREATURES].emplace_back(p);
            }
            if(r->caste_flag(FISHABLE)){
                m_raw_prefs[CREATURES_FISHABLE].emplace_back(p);
            }
            if(r->caste_flag(TRAINABLE)){
                m_raw_prefs[CREATURES_TRAINABLE].emplace_back(p);
            }
            if(r->caste_flag(MILKABLE)){
                m_raw_prefs[CREATURES_MILKABLE].emplace_back(p);
            }
            if(r->caste_flag(SHEARABLE)){
                m_raw_prefs[CREATURES_SHEARABLE].emplace_back(p);
            }
            if(r->caste_flag(BUTCHERABLE)){
                m_raw_prefs[CREATURES_BUTCHER].emplace_back(p);
            }
            if(r->caste_flag(HAS_EXTRACTS)){
                if(r->caste_flag(FISHABLE)){
                    m_raw_prefs[CREATURES_EXTRACTS_FISH].emplace_back(p);
                }else{
                    m_raw_prefs[CREATURES_EXTRACTS].emplace_back(p);
                }
            }

            for (Material *m: r->get_creature_materials().values()) {
                for (auto t: {std::make_tuple(LEATHER, LEATHERS),
                              std::make_tuple(YARN, FABRICS),
                              std::make_tuple(SILK, FABRICS)}) {
                    auto flag = std::get<0>(t);
                    auto cat = std::get<1>(t);
                    if (m->flags().has_flag(flag)) {
                        auto p = std::make_shared<ExactMaterialRolePreference>(m, SOLID);
                        m_raw_prefs[cat].emplace_back(p);
                    }
                }
                if (m->has_reaction("PARCHMENT")) {
                    auto p = std::make_shared<ExactMaterialRolePreference>(m, PRESSED);
                    m_raw_prefs[PARCHMENTS].emplace_back(p);
                }
            }
        }

        // Items
        auto item_list = m_df->get_all_item_defs();
        for(int idx=0; idx < NUM_OF_ITEM_TYPES; idx++){
            ITEM_TYPE itype = static_cast<ITEM_TYPE>(idx);

            if(item_ignore.find(itype) == item_ignore.end()){
                QString name = Item::get_item_name_plural(itype);

                bool is_armor_type = Item::is_armor_type(itype,false);

                //specific items
                int count = item_list.value(itype).count();
                if(count > 1){
                    QStringList added_subtypes;
                    //create a node for the specific item type
                    m_item_prefs.emplace_back(std::piecewise_construct,
                            std::forward_as_tuple(name),
                            std::forward_as_tuple());
                    auto cat_index = m_item_prefs.size()-1;
                    //check for clothing
                    if(is_armor_type){
                        m_item_prefs.emplace_back(std::piecewise_construct,
                                std::forward_as_tuple(Item::get_item_clothing_name(itype)),
                                std::forward_as_tuple());
                        auto clothing_cat_index = m_item_prefs.size()-1;
                        for(int sub_id = 0; sub_id < count; sub_id++){
                            ItemSubtype *stype = m_df->get_item_subtype(itype,sub_id);
                            auto p = std::make_shared<ExactItemRolePreference>(stype);

                            if(added_subtypes.contains(p->get_name()))
                                continue;
                            added_subtypes.append(p->get_name());

                            if(stype->flags().has_flag(IS_ARMOR))
                                m_item_prefs[cat_index].second.emplace_back(p);
                            if(stype->flags().has_flag(IS_CLOTHING))
                                m_item_prefs[clothing_cat_index].second.emplace_back(p);
                        }
                    }else{
                        for(int sub_id = 0; sub_id < count; sub_id++){
                            auto name = capitalize(m_df->get_preference_item_name(itype,sub_id));
                            auto p = std::make_shared<ExactItemRolePreference>(name, itype);
                            m_item_prefs[cat_index].second.emplace_back(p);
                        }
                    }
                }
            }
        }

        // Weapons
        foreach(ItemSubtype *i, m_df->get_item_subtypes(WEAPON)){
            ItemWeaponSubtype *w = qobject_cast<ItemWeaponSubtype*>(i);
            auto p = std::make_shared<ExactItemRolePreference>(w); //unfortunately a crescent halberd != halberd
            if(w->flags().has_flag(ITEMS_WEAPON_RANGED)){
                m_raw_prefs[WEAPONS_RANGED].emplace_back(p);
            }else{
                m_raw_prefs[WEAPONS_MELEE].emplace_back(p);
            }
        }

        m_loaded_raws = true;
        endResetModel();
    });// end concurrent run
    watcher.setFuture(future);

    progress.exec();
}

QModelIndex RolePreferenceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) { // preference item
        // store the parent index (starting at 1, 0 is reserved for categories) in the internalId
        return createIndex(row, column, static_cast<quintptr>(1+parent.row()));
    }
    else { // category item
        return createIndex(row, column, static_cast<quintptr>(0));
    }
}

QModelIndex RolePreferenceModel::parent(const QModelIndex &index) const
{
    int parent_row = (int)index.internalId() - 1;
    if (parent_row == -1) // category item
        return QModelIndex();
    // preference item
    return createIndex(parent_row, 0, static_cast<quintptr>(0));
}

int RolePreferenceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.internalId() != 0) // preference item
            return 0;
        // category item
        return select_category<int>(parent.row(),
                [] (const auto &cat) { return cat.size(); });
    }
    else { // root item
        return m_general_prefs.size() + m_raw_prefs.size() + m_item_prefs.size();
    }
}

int RolePreferenceModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QVariant RolePreferenceModel::data(const QModelIndex &index, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    int parent_row = (int)index.internalId() - 1;
    if (parent_row == -1) { // category item
        return get_category_name(index.row());
    }
    else { // preference item
        return select_category<QVariant>(parent_row,
                [&index] (const auto &cat) { return cat[index.row()]->get_name(); });
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
    if (!index.isValid() || index.internalId() == 0) // invalid or category item
        return nullptr;
    return select_category<RolePreference *>(index.internalId()-1,
                [&index] (const auto &cat) { return cat[index.row()].get(); });
}

QString RolePreferenceModel::get_category_name(std::size_t index) const
{
        if (index < m_general_prefs.size())
            return QString("~%1").arg(get_category_name(static_cast<GenericCategory>(index)));
        else if ((index -= m_general_prefs.size()) < m_raw_prefs.size())
            return get_category_name(static_cast<ExactCategory>(index));
        else if ((index -= m_raw_prefs.size()) < m_item_prefs.size())
            return m_item_prefs[index].first;
        else
            return QString();
}
