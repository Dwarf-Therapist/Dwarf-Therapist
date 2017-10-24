/*
Dwarf Therapist
Copyright (c) 2017 Clément Vuchener

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

#include "rolepreference.h"

#include <QSettings>
#include "preference.h"
#include "races.h"
#include "plant.h"
#include "itemsubtype.h"
#include "itemarmorsubtype.h"
#include "itemweaponsubtype.h"
#include "material.h"
#include "item.h"

RolePreference::RolePreference(PREF_TYPES type, const QString &name)
    : m_type(type)
    , m_name(name)
{
}

RolePreference::RolePreference(PREF_TYPES type, const QString &name, const std::set<int> &flags)
    : m_type(type)
    , m_name(name)
    , m_flags(flags)
{
}

RolePreference::~RolePreference() noexcept {
}

bool RolePreference::match(const Preference *p, const Dwarf *) const
{
    return m_type == p->get_pref_category();
}

std::unique_ptr<RolePreference> RolePreference::parse(QSettings &s, bool &updated)
{
    QString id;
    if (s.contains("name")) {
        id = s.value("name").toString();
    }
    else {
        LOGW << "Missing name in role preference" << s.fileName() << s.group();
        id = "Unknown";
    }
    bool is_neg = false;
    if(!id.isEmpty() && id.indexOf("-") >= 0){
        id.replace("-","");
        is_neg = true;
    }

    auto pref_type = static_cast<PREF_TYPES>(s.value("pref_category",-1).toInt());
    auto item_type = static_cast<ITEM_TYPE>(s.value("item_type",-1).toInt());
    auto exact = s.value("exact",false).toBool();
    auto mat_state = static_cast<MATERIAL_STATES>(s.value("mat_state", ANY_STATE).toInt ());

    int flag_count = s.beginReadArray("flags");
    std::set<int> flags;
    for(int i = 0; i < flag_count; i++){
        s.setArrayIndex(i);
        flags.insert(s.value("flag").toInt());
    }
    s.endArray();

    //check and update any missing armor/clothing flags
    if (Item::is_armor_type(item_type) &&
            flags.find(IS_ARMOR) == flags.end() &&
            flags.find(IS_CLOTHING) == flags.end()){
        flags.insert(IS_ARMOR);
        updated = true;
    }

    //add missing trade goods flags
    if (Item::is_trade_good(item_type) && flags.find(IS_TRADE_GOOD) == flags.end()){
        flags.insert(IS_TRADE_GOOD);
        updated = true;
    }

    //add missing trainable, remove old flags
    if (pref_type == LIKE_CREATURE) {
        decltype(flags)::iterator hunting, war;
        if ((hunting = flags.find(TRAINABLE_HUNTING)) != flags.end() &&
                (war = flags.find(TRAINABLE_WAR)) != flags.end() &&
                flags.find(TRAINABLE) == flags.end()) {
            flags.insert(TRAINABLE);
            flags.erase(hunting);
            flags.erase(war);
            updated = true;
        }
    }

    //update any general preference material names (eg. Horn -> Horn/Hoof)
    if(item_type == NONE && !exact && pref_type == LIKE_MATERIAL && !flags.empty()) {
        auto first_flag = static_cast<MATERIAL_FLAGS>(*flags.begin());
        if (first_flag < NUM_OF_MATERIAL_FLAGS) {
            QString new_name = Material::get_material_flag_desc(first_flag);
            if (new_name != id){
                id = new_name;
                updated = true;
            }
        }
    }

    //update old outdoor preference category (9) to new (99)
    if(pref_type == 9 && item_type == -1 && flags.find(999) != flags.end()){
        pref_type = LIKE_OUTDOORS;
        updated = true;
    }

    std::unique_ptr<RolePreference> p;
    if (exact) {
        if (pref_type == LIKE_ITEM && item_type != -1)
            p = std::make_unique<ExactItemRolePreference>(id, item_type, flags);
        else if (pref_type == LIKE_MATERIAL)
            p = std::make_unique<ExactMaterialRolePreference>(id, mat_state, flags);
        else
            p = std::make_unique<ExactRolePreference>(pref_type, id, flags);
    }
    else {
        if (pref_type == LIKE_ITEM && item_type != -1)
            p = std::make_unique<GenericItemRolePreference>(id, item_type, flags);
        else if (pref_type == LIKE_MATERIAL) {
            if (s.contains("mat_reaction"))
                p = std::make_unique<MaterialReactionRolePreference>(id, mat_state, s.value("mat_reaction").toString(), flags);
            else
                p = std::make_unique<GenericMaterialRolePreference>(id, mat_state, flags);
        }
        else
            p = std::make_unique<GenericRolePreference>(pref_type, id, flags);
    }

    p->aspect.weight = s.value("weight",1.0).toFloat();
    if (p->aspect.weight < 0)
        p->aspect.weight = 1.0;
    p->aspect.is_neg = is_neg;

    return p;
}

void RolePreference::write(QSettings &s) const {
    s.setValue("pref_category",QString::number((int)m_type));
    s.setValue("exact", false); // will be overwritten by subclass if necessary

    QString id = QString("%1%2").arg(aspect.is_neg ? "-" : "").arg(m_name);
    s.setValue("name",id);
    if(aspect.weight != 1.0)
        s.setValue("weight",QString::number(aspect.weight,'g',2));

    if (!m_flags.empty()) {
        s.beginWriteArray("flags", m_flags.size());
        int i = 0;
        for (auto f: m_flags) {
            s.setArrayIndex(i++);
            s.setValue("flag", f);
        }
        s.endArray();
    }
}

std::unique_ptr<RolePreference> RolePreference::copy() const {
    return std::make_unique<RolePreference>(*this);
}

ItemRolePreference::ItemRolePreference(ITEM_TYPE item_type)
    : m_item_type(item_type)
{
}

void ItemRolePreference::write(QSettings &s) const {
    s.setValue("item_type", QString::number((int)m_item_type));
}

bool ItemRolePreference::match(const Preference *p, const Dwarf *d) const {
    auto *ip = dynamic_cast<const ItemPreference *>(p);
    return ip &&
        ip->get_item_type() == m_item_type &&
        (!d || ip->can_wield(d));
}

MaterialRolePreference::MaterialRolePreference(MATERIAL_STATES mat_state)
    : m_mat_state(mat_state)
{
}

void MaterialRolePreference::write(QSettings &s) const {
    if (m_mat_state != ANY_STATE)
        s.setValue("mat_state", QString::number(m_mat_state));
}

bool MaterialRolePreference::match(const Preference *p) const
{
    if (m_mat_state == ANY_STATE)
        return true;
    auto *mp = dynamic_cast<const MaterialPreference *>(p);
    return mp && m_mat_state == mp->get_mat_state();
}

ExactRolePreference::ExactRolePreference(PREF_TYPES type, const QString &name, const std::set<int> &flags)
    : RolePreference(type, name, flags)
{
}

static std::set<int> to_set(const FlagArray &flags) {
    std::set<int> flag_set;
    for (auto f: flags.active_flags())
        flag_set.insert(f);
    return flag_set;
}

ExactRolePreference::ExactRolePreference(const Race *r)
    : RolePreference(LIKE_CREATURE,
                     r->plural_name(),
                     to_set(CreaturePreference::select_flags(r)))
{
}

ExactRolePreference::ExactRolePreference(const Plant *p)
    : RolePreference(LIKE_PLANT, p->name_plural())
{
    if(!p->flags().has_flag(P_SAPLING) && !p->flags().has_flag(P_TREE)){
        for (auto f: {P_DRINK, P_MILL, P_HAS_EXTRACTS}) {
            if (p->flags().has_flag(f))
                m_flags.insert(f);
        }
        if (p->flags().has_flag(P_CROP)) {
            m_flags.insert(P_CROP);
            if (p->flags().has_flag(P_SEED))
                m_flags.insert(P_SEED);
        }
    }
}

void ExactRolePreference::write(QSettings &s) const {
    RolePreference::write(s);
    s.setValue("exact", true);
}

std::unique_ptr<RolePreference> ExactRolePreference::copy() const {
    return std::make_unique<ExactRolePreference>(*this);
}

bool ExactRolePreference::match(const Preference *p, const Dwarf *d) const {
    return RolePreference::match(p, d) &&
        QString::compare(get_name(), p->get_name(), Qt::CaseInsensitive) == 0;
}

ExactItemRolePreference::ExactItemRolePreference(const QString &name, ITEM_TYPE type, const std::set<int> &flags)
    : ExactRolePreference(LIKE_ITEM, name, flags)
    , ItemRolePreference(type)
{
}

static std::set<int> item_flags(const ItemSubtype *i) {
    std::set<int> flags;
    if (auto w = dynamic_cast<const ItemWeaponSubtype*>(i)) {
        for (auto f: {ITEMS_WEAPON, ITEMS_WEAPON_RANGED}) {
            if (w->flags().has_flag(f))
                flags.insert(f);
        }
    }
    if (auto a = dynamic_cast<const ItemArmorSubtype*>(i)) {
        for (auto f: {IS_ARMOR, IS_CLOTHING}) {
            if (a->flags().has_flag(f))
                flags.insert(f);
        }
    }
    return flags;
}

ExactItemRolePreference::ExactItemRolePreference(const ItemSubtype *i)
    : ExactRolePreference(LIKE_ITEM, i->name_plural(), item_flags(i))
    , ItemRolePreference(i->type())
{
}

void ExactItemRolePreference::write(QSettings &s) const {
    ExactRolePreference::write(s);
    ItemRolePreference::write(s);
}

std::unique_ptr<RolePreference> ExactItemRolePreference::copy() const {
    return std::make_unique<ExactItemRolePreference>(*this);
}

bool ExactItemRolePreference::match(const Preference *p, const Dwarf *d) const {
    return ItemRolePreference::match(p, d) && ExactRolePreference::match(p, d);
}

GenericRolePreference::GenericRolePreference(PREF_TYPES type, const QString &name, const std::set<int> &flags)
    : RolePreference(type, name, flags)
{
}

ExactMaterialRolePreference::ExactMaterialRolePreference(const QString &name, MATERIAL_STATES state, const std::set<int> &flags)
    : ExactRolePreference(LIKE_MATERIAL, name, flags)
    , MaterialRolePreference(state)
{
}

ExactMaterialRolePreference::ExactMaterialRolePreference(const Material *m, MATERIAL_STATES state)
    : ExactRolePreference(LIKE_MATERIAL, m->get_material_name(state).trimmed())
    , MaterialRolePreference(state)
{
    for (auto f: {IS_GEM, IS_GLASS, CRYSTAL_GLASSABLE, IS_METAL,
                  IS_WOOD, THREAD_PLANT, IS_DYE, ITEMS_DELICATE}) {
        if (m->flags().has_flag(f))
            m_flags.insert(f);
    }
    if (m->flags().has_flag(IS_STONE)) {
        m_flags.insert(IS_STONE);
        if (m->flags().has_flag(ITEMS_QUERN) && m->flags().has_flag(NO_STONE_STOCKPILE)) {
            m_flags.insert(ITEMS_QUERN);
            m_flags.insert(NO_STONE_STOCKPILE);
        }
    }
}

void ExactMaterialRolePreference::write(QSettings &s) const {
    ExactRolePreference::write(s);
    MaterialRolePreference::write(s);
}

std::unique_ptr<RolePreference> ExactMaterialRolePreference::copy() const {
    return std::make_unique<ExactMaterialRolePreference>(*this);
}

bool ExactMaterialRolePreference::match(const Preference *p, const Dwarf *d) const {
    return MaterialRolePreference::match(p) && ExactRolePreference::match(p, d);
}

std::unique_ptr<RolePreference> GenericRolePreference::copy() const {
    return std::make_unique<GenericRolePreference>(*this);
}

bool GenericRolePreference::match(const Preference *p, const Dwarf *d) const {
    return RolePreference::match(p, d) &&
        std::all_of(m_flags.begin(), m_flags.end(),
                    [p] (auto f) { return p->has_flag(f); });
}

GenericMaterialRolePreference::GenericMaterialRolePreference(const QString &name, MATERIAL_STATES state, const std::set<int> &flags)
    : GenericRolePreference(LIKE_MATERIAL, name, flags)
    , MaterialRolePreference(state)
{
}

void GenericMaterialRolePreference::write(QSettings &s) const {
    GenericRolePreference::write(s);
    MaterialRolePreference::write(s);
}

std::unique_ptr<RolePreference> GenericMaterialRolePreference::copy() const {
    return std::make_unique<GenericMaterialRolePreference>(*this);
}

bool GenericMaterialRolePreference::match(const Preference *p, const Dwarf *d) const {
    return MaterialRolePreference::match(p) && GenericRolePreference::match(p, d);
}

GenericItemRolePreference::GenericItemRolePreference(const QString &name, ITEM_TYPE item_type, const std::set<int> &flags)
    : GenericRolePreference(LIKE_ITEM, name, flags)
    , ItemRolePreference(item_type)
{
}

void GenericItemRolePreference::write(QSettings &s) const {
    GenericRolePreference::write(s);
    ItemRolePreference::write(s);
}

std::unique_ptr<RolePreference> GenericItemRolePreference::copy() const {
    return std::make_unique<GenericItemRolePreference>(*this);
}

bool GenericItemRolePreference::match(const Preference *p, const Dwarf *d) const {
    return ItemRolePreference::match(p, d) && GenericRolePreference::match(p, d);
}

MaterialReactionRolePreference::MaterialReactionRolePreference(const QString &name, MATERIAL_STATES state, const QString &reaction, const std::set<int> &flags)
    : GenericMaterialRolePreference(name, state, flags)
    , m_reaction(reaction)
{
}

bool MaterialReactionRolePreference::match(const Preference *p, const Dwarf *d) const {
    if (!GenericMaterialRolePreference::match(p, d))
        return false;
    auto mp = dynamic_cast<const MaterialPreference *>(p);
    return mp && mp->get_material()->has_reaction(m_reaction);
}

void MaterialReactionRolePreference::write(QSettings &s) const {
    GenericMaterialRolePreference::write(s);
    s.setValue("mat_reaction", m_reaction);
}

std::unique_ptr<RolePreference> MaterialReactionRolePreference::copy() const {
    return std::make_unique<MaterialReactionRolePreference>(*this);
}

