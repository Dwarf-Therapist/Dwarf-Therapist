/*
Dwarf Therapist
Copyright (c) 2009 Trey Stout (chmod)

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
#ifndef DWARF_THERAPIST_H
#define DWARF_THERAPIST_H

#define DT (static_cast<DwarfTherapist *>(QCoreApplication::instance()))

#include "columntypes.h"

#include <QApplication>
#include <QVector>
#include <QSharedPointer>
#include <QVariant>
#include <QColor>
#include <memory>
#include "global_enums.h"

class QTreeWidgetItem;
class OptionsMenu;
class QSettings;
class CustomProfession;
class SuperLabor;
class Dwarf;
class Word;
class LogManager;
class CellColorDef;
class MainWindow;
class DFInstance;

class DwarfTherapist : public QApplication {
    Q_OBJECT
public:
    DwarfTherapist(int &argc, char **argv);
    virtual ~DwarfTherapist();

    QList<Dwarf*> get_dwarves();

    QList<CustomProfession*> get_custom_professions() {return m_custom_professions.values();}
    CustomProfession *get_custom_profession(QString name);
    CustomProfession *get_custom_prof_icon(int prof_id) {return m_custom_prof_icns.value(prof_id);}
    QMap<int, CustomProfession*> &get_custom_prof_icons() {return m_custom_prof_icns;}
    SuperLabor *get_super_labor(QString name){return m_super_labors.value(name);}
    QList<SuperLabor*> get_super_labors();

    MainWindow *get_main_window();
    QSettings *user_settings() {return m_user_settings.get();}
    OptionsMenu *get_options_menu() {return m_options_menu;}
    Dwarf *get_dwarf_by_id(int dwarf_id);

    void load_game_translation_tables(DFInstance *df);
    QString get_generic_word(const uint &offset) {return m_generic_words.value(offset, "UNKNOWN");}
    QString get_dwarf_word(const uint &offset) {return m_dwarf_words.value(offset, get_generic_word(offset));}
    Word * get_word(const uint & offset) { return m_language.value(offset, NULL); }

    bool labor_cheats_allowed() const {return m_allow_labor_cheats;}
    bool hide_non_adults() const {return m_hide_non_adults;}
    bool hide_non_citizens() const {return m_hide_non_citizens;}
    bool show_labor_roles() const {return m_show_labor_roles;}
    bool show_skill_roles() const {return m_show_skill_roles;}
    bool format_SI() const {return m_use_SI;}

    LogManager *get_log_manager() {return m_log_mgr;}
    DFInstance *get_DFInstance();

    QSharedPointer<CellColorDef> get_global_color(GLOBAL_COLOR_TYPES gc_type);
    QColor get_happiness_color(DWARF_HAPPINESS h) {return m_happiness_colors.value(h,QColor(Qt::transparent));}

    bool multiple_castes() const {return m_multiple_castes;}
    void multiple_castes(bool val) {m_multiple_castes = val;}
    bool show_skill_learn_rates() const {return m_show_skill_learn_rates;}
    void show_skill_learn_rates(const bool val) {m_show_skill_learn_rates = val;}
    bool arena_mode() const {return m_arena_mode;}

    void emit_settings_changed();
    void emit_roles_changed();
    void emit_customizations_changed();
    void emit_labor_counts_updated();
    void update_specific_header(int id, COLUMN_TYPE type);

    QString project_homepage() {return m_url_homepage;}

public slots:
    int add_custom_profession(Dwarf *d = 0);
    void add_custom_professions(QList<CustomProfession*> cps);
    void add_custom_profession(CustomProfession *cp);
    void add_super_labor(Dwarf *d = 0);

    void update_multilabor(Dwarf *d, QString name, CUSTOMIZATION_TYPE cType);

    void read_settings();
    void load_customizations();

    void write_settings();
    void write_custom_professions();
    void write_super_labors();

    void import_existing_professions();
    void edit_customization();
    void edit_customization(QTreeWidgetItem *);
    void delete_customization();
    void emit_units_refreshed();

private:
    struct customization_data{
        QVariant id;
        CUSTOMIZATION_TYPE type;
    };

    QVector<QString> m_generic_words;
    QVector<QString> m_dwarf_words;
    QVector<Word *> m_language;
    QMap<QString,CustomProfession*> m_custom_professions;
    QMap<int, CustomProfession*> m_custom_prof_icns;
    QMap<QString,SuperLabor*> m_super_labors;
    std::unique_ptr<QSettings> m_user_settings;
    MainWindow *m_main_window;
    OptionsMenu *m_options_menu;

    bool m_allow_labor_cheats;
    bool m_hide_non_adults;
    bool m_hide_non_citizens;
    bool m_show_labor_roles;
    bool m_show_skill_roles;
    bool m_use_SI;

    bool m_multiple_castes;
    bool m_show_skill_learn_rates;
    bool m_arena_mode;

    LogManager *m_log_mgr;
    QHash<GLOBAL_COLOR_TYPES,QSharedPointer<CellColorDef> > m_colors;
    QHash<DWARF_HAPPINESS,QColor> m_happiness_colors;

    static const QString m_url_homepage;

    void setup_search_paths();
    void setup_logging(const QString &path, bool debug_logging, bool trace_logging);
    void load_translator();
    void edit_customization(QList<QVariant> data);
    void check_global_color(GLOBAL_COLOR_TYPES key, QString setting_key, QString title, QString desc, QColor col_default);

    customization_data build_c_data(QVariant);

    //these should be used when working with custom professions to keep them in sync with
    //their related automatically generated super labors
    void del_custom_profession(CustomProfession *cp);
    void ins_custom_profession(CustomProfession *cp);

signals:
    void settings_changed();
    void roles_changed();
    void labor_counts_updated();
    void customizations_changed();
    void units_refreshed(); //raised by the model object after a read is completed
    void connected();
};

#endif
