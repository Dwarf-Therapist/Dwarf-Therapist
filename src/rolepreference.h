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
#ifndef ROLE_PREFERENCE_H
#define ROLE_PREFERENCE_H

#include <memory>
#include <set>
#include "global_enums.h"

class QSettings;
class Preference;
class Race;
class Plant;
class ItemSubtype;
class Material;
class Dwarf;

class RolePreference {
public:
    RolePreference(PREF_TYPES type, const QString &name);
    virtual ~RolePreference() noexcept;

    PREF_TYPES get_pref_category() const { return m_type; }
    const QString &get_name() const { return m_name; }

    virtual bool match(const Preference *p, const Dwarf *d) const;

    static std::unique_ptr<RolePreference> parse(QSettings &s, bool &updated);
    virtual void write(QSettings &s) const;
    virtual std::unique_ptr<RolePreference> copy() const;

private:
    PREF_TYPES m_type;
    QString m_name;

protected:
    RolePreference(PREF_TYPES type, const QString &name, const std::set<int> &flags);
    //these flags are used when writing to the ini. Exact meaning depends on the subclass.
    std::set<int> m_flags;
};

// common interface for item-based role preference, not an actual role preference
class ItemRolePreference
{
public:
    ITEM_TYPE get_item_type() const { return m_item_type; }

protected:
    ItemRolePreference(ITEM_TYPE item_type);

    void write(QSettings &s) const;

    bool match(const Preference *p, const Dwarf *d) const;

private:
    ITEM_TYPE m_item_type;
};

// common interface for material-based role preference, not an actual role preference
class MaterialRolePreference
{
public:
    MATERIAL_STATES get_mat_state() const { return m_mat_state; }

protected:
    MaterialRolePreference(MATERIAL_STATES mat_state);

    void write(QSettings &s) const;

    bool match(const Preference *p) const;

private:
    MATERIAL_STATES m_mat_state;
};

class ExactRolePreference: public RolePreference {
public:
    // Note: ExactRolePreference stores flags but they are not actually used in current code.

    template<typename... Flags>
    ExactRolePreference(PREF_TYPES type, const QString &name, Flags... flags)
        : RolePreference(type, name, {flags...}) { }
    ExactRolePreference(PREF_TYPES type, const QString &name, const std::set<int> &flags);
    ExactRolePreference(const Race *r); // LIKE_CREATURE
    ExactRolePreference(const Plant *p); // LIKE_PLANT

    bool match(const Preference *p, const Dwarf *d) const override;

    void write(QSettings &s) const override;
    std::unique_ptr<RolePreference> copy() const override;
};

class ExactItemRolePreference: public ExactRolePreference, public ItemRolePreference {
public:
    template<typename... Flags>
    ExactItemRolePreference(const QString &name, ITEM_TYPE type, Flags... flags)
        : ExactRolePreference(LIKE_ITEM, name, {flags...})
        , ItemRolePreference(type)
    {
    }
    ExactItemRolePreference(const QString &name, ITEM_TYPE type, const std::set<int> &flags);
    ExactItemRolePreference(const ItemSubtype *i);

    bool match(const Preference *p, const Dwarf *d) const override;

    void write(QSettings &s) const override;
    std::unique_ptr<RolePreference> copy() const override;
};

class ExactMaterialRolePreference: public ExactRolePreference, public MaterialRolePreference {
public:
    ExactMaterialRolePreference(const QString &name, MATERIAL_STATES state, const std::set<int> &flags);
    ExactMaterialRolePreference(const Material *m, MATERIAL_STATES state);

    bool match(const Preference *p, const Dwarf *d) const override;

    void write(QSettings &s) const override;
    std::unique_ptr<RolePreference> copy() const override;
};

class GenericRolePreference: public RolePreference {
public:
    template<typename... Flags>
    GenericRolePreference(PREF_TYPES type, const QString &name, Flags... flags)
        : RolePreference(type, name, {flags...}) { }
    GenericRolePreference(PREF_TYPES type, const QString &name, const std::set<int> &flags = {});

    bool match(const Preference *p, const Dwarf *d) const override;

    std::unique_ptr<RolePreference> copy() const override;
};

class GenericMaterialRolePreference: public GenericRolePreference, public MaterialRolePreference {
public:
    template<typename... Flags>
    GenericMaterialRolePreference(const QString &name, MATERIAL_STATES state, Flags... flags)
        : GenericRolePreference(LIKE_MATERIAL, name, {flags...})
        , MaterialRolePreference(state)
    {
    }
    GenericMaterialRolePreference(const QString &name, MATERIAL_STATES state, const std::set<int> &flags);

    bool match(const Preference *p, const Dwarf *d) const override;

    void write(QSettings &s) const override;
    std::unique_ptr<RolePreference> copy() const override;
};

class GenericItemRolePreference: public GenericRolePreference, public ItemRolePreference {
public:
    template<typename... Flags>
    GenericItemRolePreference(const QString &name, ITEM_TYPE item_type, Flags... flags)
        : GenericRolePreference(LIKE_ITEM, name, {flags...})
        , ItemRolePreference(item_type)
    {
    }
    GenericItemRolePreference(const QString &name, ITEM_TYPE item_type, const std::set<int> &flags);

    bool match(const Preference *p, const Dwarf *d) const override;

    void write(QSettings &s) const override;
    std::unique_ptr<RolePreference> copy() const override;
};

class MaterialReactionRolePreference: public GenericMaterialRolePreference {
public:
    template<typename... Flags>
    MaterialReactionRolePreference(const QString &name, MATERIAL_STATES state, const QString &reaction, Flags... flags)
        : GenericMaterialRolePreference(name, state, flags...)
        , m_reaction(reaction)
    {
    }
    MaterialReactionRolePreference(const QString &name, MATERIAL_STATES state, const QString &reaction, const std::set<int> &flags);

    bool match(const Preference *p, const Dwarf *d) const override;

    void write(QSettings &s) const override;
    std::unique_ptr<RolePreference> copy() const override;

private:
    QString m_reaction;
};

#endif // ROLE_PREFERENCE_H
