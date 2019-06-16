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

#include "dwarftherapist.h"
#include "dfinstance.h"
#include "mainwindow.h"
#include "optionsmenu.h"
#include "version.h"
#include "defines.h"
#include "customprofession.h"
#include "superlabor.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "dwarf.h"
#include "word.h"
#include "ui_mainwindow.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"
#include "viewmanager.h"
#include "dwarfstats.h"
#include "defaultfonts.h"
#include "dtstandarditem.h"
#include "cellcolordef.h"
#include "standardpaths.h"
#include <QMessageBox>
#include <QSettings>
#include <QToolTip>
#include <QTranslator>
#include <QTimer>
#include <QCommandLineParser>

const QString DwarfTherapist::m_url_homepage = QString("https://github.com/%1/%2").arg(REPO_OWNER).arg(REPO_NAME);

DwarfTherapist::DwarfTherapist(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_main_window(0)
    , m_options_menu(0)
    , m_allow_labor_cheats(false)
    , m_hide_non_adults(false)
    , m_hide_non_citizens(false)
    , m_show_labor_roles(true)
    , m_show_skill_roles(true)
    , m_use_SI(true)
    , m_multiple_castes(false)
    , m_show_skill_learn_rates(false)
    , m_arena_mode(false) //manually set this to true to do arena testing (very hackish, all units will be animals)
    , m_log_mgr(0)
{
#ifdef Q_OS_LINUX
    // Linux prefers lower case and no space in package names.
    setApplicationName("dwarftherapist");
#else
    setApplicationName("Dwarf Therapist");
#endif
    // Do not set organization name to avoid QStandardPaths using
    // paths like share/dwarftherapist/dwarftherapist/...
#ifdef Q_OS_OSX
    setOrganizationDomain("io.github.dwarf-therapist");
#endif
    setApplicationDisplayName("Dwarf Therapist");
    Version v; // current version
    setApplicationVersion(v.to_string());

    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption debug_option("debug", tr("Set logging to debug level."));
    parser.addOption(debug_option);
    QCommandLineOption trace_option("trace", tr("Set logging to trace level."));
    parser.addOption(trace_option);
    QCommandLineOption portable_option("portable", tr("Start in portable mode (look for data and config files relatively to the executable)."));
    parser.addOption(portable_option);
    QCommandLineOption devmode_option("devmode", tr("Start in developer mode (look for data and config files relatively to the executable and for static data in <source_datadir>)."), tr("source_datadir"));
    parser.addOption(devmode_option);
    parser.process(*this);

    setup_logging(parser.isSet(debug_option), parser.isSet(trace_option));
    load_translator();
    {
        auto mode = StandardPaths::DefaultMode;
        if (parser.isSet(devmode_option))
            mode = StandardPaths::Mode::Developer;
        else if (parser.isSet(portable_option))
            mode = StandardPaths::Mode::Portable;
        StandardPaths::init_paths(mode, parser.value(devmode_option));
    }

    TRACE << "Creating settings object";
    m_user_settings = StandardPaths::settings();

    TRACE << "Creating options menu";
    m_options_menu = new OptionsMenu;

    TRACE << "Creating main window";
    m_main_window = new MainWindow;
    m_options_menu->setParent(m_main_window, Qt::Dialog);

    TRACE << "connecting signals";
    //connect(m_options_menu, SIGNAL(settings_changed()), SIGNAL(settings_changed())); // the telephone game...
    connect(m_options_menu, SIGNAL(settings_changed()), this, SLOT(read_settings()));
    connect(m_main_window->ui->act_options, SIGNAL(triggered()), m_options_menu, SLOT(exec()));
    connect(m_main_window->ui->act_import_existing_professions, SIGNAL(triggered()), this, SLOT(import_existing_professions()));
    connect(m_main_window->ui->tree_custom_professions, SIGNAL(itemActivated(QTreeWidgetItem*,int)), this, SLOT(edit_customization(QTreeWidgetItem*)));
    connect(m_main_window->ui->act_add_custom_profession, SIGNAL(triggered()), this, SLOT(add_custom_profession()));
    connect(m_main_window->ui->act_add_super_labor, SIGNAL(triggered()), this, SLOT(add_super_labor()));
    connect(m_main_window->ui->le_filter_text, SIGNAL(textChanged(const QString&)), m_main_window->get_proxy(), SLOT(setFilterFixedString(const QString&)));

    read_settings();
    load_customizations();

    bool read = m_user_settings->value("options/read_on_startup", true).toBool();
    if (read) {
        QTimer::singleShot(0, m_main_window, SLOT(connect_to_df()));
    }
    m_main_window->show();
}

DwarfTherapist::~DwarfTherapist(){
    UnitHealth::cleanup();

    qDeleteAll(m_language);
    m_language.clear();
    qDeleteAll(m_custom_prof_icns);
    m_custom_prof_icns.clear();
    qDeleteAll(m_custom_professions);
    m_custom_professions.clear();
    qDeleteAll(m_super_labors);
    m_super_labors.clear();

    delete m_options_menu;
    delete m_main_window;
    delete m_log_mgr;
}

MainWindow* DwarfTherapist::get_main_window(){
    return m_main_window;
}

DFInstance* DwarfTherapist::get_DFInstance(){
    return m_main_window->get_DFInstance();
}

void DwarfTherapist::setup_logging(bool debug_logging, bool trace_logging) {
    LOG_LEVEL min_level = LL_INFO;

#ifdef QT_DEBUG
    min_level = LL_DEBUG;
#endif

    if (trace_logging) {
        min_level = LL_TRACE;
    } else if (debug_logging) {
        min_level = LL_DEBUG;
    }

    //setup logging
    m_log_mgr = new LogManager(this);
    TruncatingFileLogger *log = m_log_mgr->add_logger(
            #ifdef Q_OS_LINUX
                ""
            #else
                "log/run.log"
            #endif
                );
    if (log) {
        LogAppender *app = m_log_mgr->add_appender("core", log, LL_TRACE);
        if (app) {
            Version v; // current version
            LOGI << "Dwarf Therapist" << v.to_string() << "starting normally.";
            LOGI << "Runtime QT Version" << qVersion();
            //app->set_minimum_level(min_level);
            app->set_minimum_level(min_level);
        } else {
            qCritical() << "Could not open logfile!";
            qApp->exit(1);
        }
    } else {
        qCritical() << "Could not open logfile!";
        qApp->exit(1);
    }
}

QList<Dwarf*> DwarfTherapist::get_dwarves(){
    return m_main_window->get_model()->get_dwarves();
}

void DwarfTherapist::load_translator() {
    TRACE << "loading translations";
    QTranslator translator;
    translator.load("dwarftherapist_en");
    installTranslator(&translator);
    TRACE << "english translation loaded";
}

void DwarfTherapist::read_settings() {
    LOGI << "beginning to read settings";

    // HACK!
    if (m_user_settings->value("it_feels_like_the_first_time", true).toBool() ||
            !m_user_settings->contains("options/colors/happiness/1") ||
            !m_user_settings->contains("options/colors/nobles/1")) {
        m_options_menu->write_settings(); //write it out so that we can get default colors loaded
        emit settings_changed(); // this will cause delegates to get the right default colors
        m_user_settings->setValue("it_feels_like_the_first_time", false);
    }

    m_user_settings->beginGroup("options");
    if (m_user_settings->value("show_toolbutton_text", true).toBool()) {
        m_main_window->get_toolbar()->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    } else {
        m_main_window->get_toolbar()->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    m_allow_labor_cheats = m_user_settings->value("allow_labor_cheats", false).toBool();
    m_hide_non_adults = m_user_settings->value("hide_children_and_babies",false).toBool();
    m_hide_non_citizens = m_user_settings->value("hide_non_citizens",false).toBool();
    m_show_labor_roles = m_user_settings->value("show_roles_in_labor",true).toBool();
    m_show_skill_roles = m_user_settings->value("show_roles_in_skills",true).toBool();
    m_use_SI = m_user_settings->value("SI_formatting",true).toBool();

    //refresh global colors
    m_user_settings->beginGroup("colors");
    check_global_color(GCOL_ACTIVE,"active_labor",tr("Active"),tr("Color when the related action is enabled and active."),QColor(78,78,179));
    check_global_color(GCOL_PENDING,"pending_color",tr("Pending"),tr("Color when an action has been flagged to be enabled, but hasn't been yet."),QColor(203,174,40));
    check_global_color(GCOL_DISABLED,"disabled_color",tr("Disabled"),tr("Color of the cell when the action cannot be toggled."),QColor(187,34,34,125));

    m_user_settings->beginGroup("happiness");
    foreach(QString k, m_user_settings->childKeys()) {
        DWARF_HAPPINESS h = static_cast<DWARF_HAPPINESS>(k.toInt());
        m_happiness_colors[h] = m_user_settings->value(k).value<QColor>();
    }
    m_user_settings->endGroup();//happiness
    m_user_settings->endGroup();//colors

    //set the application fonts
    QApplication::setFont(DT->user_settings()->value("main_font", QFont(DefaultFonts::getMainFontName(), DefaultFonts::getMainFontSize())).value<QFont>());
    QToolTip::setFont(DT->user_settings()->value("tooltip_font", QFont(DefaultFonts::getTooltipFontName(), DefaultFonts::getTooltipFontSize())).value<QFont>());

    //set a variable we'll use in the dwarfstats for role calcs
    DwarfStats::set_att_potential_weight(DT->user_settings()->value("default_attribute_potential_weight",0.5f).toFloat());
    DwarfStats::set_skill_rate_weight(DT->user_settings()->value("default_skill_rate_weight",0.25f).toFloat());

    //notify items if they should show tooltips
    DTStandardItem::set_show_tooltips(DT->user_settings()->value("grid/show_tooltips",true).toBool());

    m_user_settings->endGroup();
    LOGI << "finished reading settings";
    //emit the settings_changed to everything else after we've refreshed our global settings
    emit settings_changed();
}

void DwarfTherapist::check_global_color(GLOBAL_COLOR_TYPES key, QString setting_key, QString title, QString desc, QColor col_default){
    QColor tmp = m_user_settings->value(setting_key,col_default).value<QColor>();
    if(!tmp.isValid()){
        tmp = col_default;
    }
    if(!m_colors.contains(key)){
        m_colors.insert(key,QSharedPointer<CellColorDef>(new CellColorDef(tmp,setting_key,title,desc)));
    }else{
        m_colors.value(key)->set_color(tmp);
    }
}

QSharedPointer<CellColorDef> DwarfTherapist::get_global_color(GLOBAL_COLOR_TYPES gc_type){
    return m_colors.value(gc_type);
}

void DwarfTherapist::load_customizations(){
    m_user_settings->beginGroup("custom_professions");
    {
        QStringList profession_names = m_user_settings->childGroups();
        foreach(QString prof, profession_names) {
            CustomProfession *cp = new CustomProfession(prof, *m_user_settings, this);
            if(cp->prof_id() > -1){
                m_custom_prof_icns.insert(cp->prof_id(), cp);//profession icon override
            }else{
                m_custom_professions.insert(cp->get_name(),cp);
            }
        }
    }
    m_user_settings->endGroup();

    //read in the custom superlabors
    int size = m_user_settings->beginReadArray("super_labors");
    {
        for(int idx = 0; idx < size; idx++) {
            m_user_settings->setArrayIndex(idx);
            SuperLabor *sl = new SuperLabor(*m_user_settings.get(),this);
            m_super_labors.insert(sl->get_name(),sl);
        }
    }
    m_user_settings->endArray();

    m_main_window->load_customizations();
}

void DwarfTherapist::emit_units_refreshed(){
    //perform anything that requires doing after the read is completed

    //emit a signal as well
    emit units_refreshed();
    emit connected();
}

void DwarfTherapist::write_settings() {
    write_custom_professions();
    write_super_labors();
}

void DwarfTherapist::write_custom_professions(){

    if (m_custom_professions.size() > 0 || m_custom_prof_icns.size() > 0) {
        m_user_settings->beginGroup("custom_professions");
        m_user_settings->remove(""); // clear all of them, so we can re-write

        CustomProfession *cp;
        //save the custom professions
        foreach(cp, m_custom_professions.values()) {
            cp->save(*m_user_settings.get());
        }
        //save the custom profession icons
        foreach(CustomProfession *cp, m_custom_prof_icns.values()){
            cp->save(*m_user_settings.get());
        }
        m_user_settings->endGroup();
    }
    emit emit_customizations_changed();
}
void DwarfTherapist::write_super_labors(){
    if (m_super_labors.size() > 0) {
        m_user_settings->remove("super_labors"); // clear all of them, so we can re-write
        m_user_settings->beginWriteArray("super_labors");
        int idx = 0;
        foreach(SuperLabor *sl, m_super_labors) {
            m_user_settings->setArrayIndex(idx++);
            sl->save(*m_user_settings.get());
        }
        m_user_settings->endArray();
    }
    emit emit_customizations_changed();
}

QList<SuperLabor*> DwarfTherapist::get_super_labors(){
    return m_super_labors.values();
}

/* CUSTOMIZATIONS (custom professions, icons, super labors) */

DwarfTherapist::customization_data DwarfTherapist::build_c_data(QVariant data){
    Q_ASSERT(data.canConvert<QVariantList>());
    DwarfTherapist::customization_data c_data;
    c_data.id = data.toList().at(0);
    c_data.type = static_cast<CUSTOMIZATION_TYPE>(data.toList().at(1).toInt());
    return c_data;
}

void DwarfTherapist::import_existing_professions() {
    int imported = 0;
    foreach(Dwarf *d, get_dwarves()) {
        QString prof = d->custom_profession_name();
        if (prof.isEmpty())
            continue;
        if (!m_custom_professions.contains(prof)) { // import it
            CustomProfession *cp = new CustomProfession(d, this);
            if(cp->prof_id() > -1)
                m_custom_prof_icns.insert(cp->prof_id(),cp);
            else
                m_custom_professions.insert(prof, cp);
            imported++;
        }
    }
    m_main_window->load_customizations();
    QMessageBox::information(m_main_window, tr("Import Successful"),
                             tr("Imported %n custom profession(s)", "", imported));
}

CustomProfession *DwarfTherapist::get_custom_profession(QString name) {
    CustomProfession *retval = m_custom_professions.value(name,0);
    return retval;
}

void DwarfTherapist::add_custom_professions(QList<CustomProfession*> cps) {
    foreach(CustomProfession *cp, cps){
        m_custom_professions.insert(cp->get_name(),cp);
    }
    m_main_window->load_customizations();
    write_custom_professions();
}

void DwarfTherapist::add_custom_profession(CustomProfession *cp) {
    m_custom_professions.insert(cp->get_name(), cp);
    ins_custom_profession(cp);
}

int DwarfTherapist::add_custom_profession(Dwarf *d) {
    CustomProfession *cp = new CustomProfession(d, this);
    int accepted = cp->show_builder_dialog(m_main_window);
    if (accepted) {
        ins_custom_profession(cp);
    }
    return accepted;
}

void DwarfTherapist::add_super_labor(Dwarf *d) {
    SuperLabor *sl = new SuperLabor(d,this);
    int accepted = sl->show_builder_dialog(m_main_window);
    if (accepted) {
        m_super_labors.insert(sl->get_name(),sl);
    }
    m_main_window->load_customizations();
    write_super_labors();
}

void DwarfTherapist::ins_custom_profession(CustomProfession *cp){
    QString name = cp->get_name();
    m_custom_professions.insert(name,cp);
    m_main_window->load_customizations();
    write_custom_professions();
}

//! from CP context menu's "Edit..."
void DwarfTherapist::edit_customization() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    edit_customization(a->data().toList());
}

//! from double-clicking a profession
void DwarfTherapist::edit_customization(QTreeWidgetItem *i) {
    if(i->childCount() > 0)
        return;
    QList<QVariant> data;
    data << i->data(0,Qt::UserRole) << i->data(0,Qt::UserRole+1);
    edit_customization(data);
}

void DwarfTherapist::update_multilabor(Dwarf *d, QString name, CUSTOMIZATION_TYPE cType){
    MultiLabor *ml;
    if(cType == CUSTOM_PROF){
        ml = m_custom_professions.take(name);
    }else{
        ml = m_super_labors.take(name);
    }
    if(ml){
        ml->set_labors(d);
        if(cType == CUSTOM_PROF)
            m_custom_professions.insert(name,qobject_cast<CustomProfession*>(ml));
        else
            m_super_labors.insert(name,qobject_cast<SuperLabor*>(ml));
        write_custom_professions();
    }
}

void DwarfTherapist::edit_customization(QList<QVariant> data){
    DwarfTherapist::customization_data c_data = build_c_data(data);
    if(c_data.type != CUSTOM_SUPER){
        CustomProfession *cp;
        if(c_data.type == CUSTOM_ICON){
            //custom icons are slightly different. if one doesn't already exist with the profession id specified, create it
            cp = m_custom_prof_icns.value(c_data.id.toInt());
            if(!cp){
                cp = new CustomProfession(c_data.id.toInt(),this);
            }
        }else{
            cp = m_custom_professions.take(c_data.id.toString());
            if (!cp) {
                LOGW << "tried to edit custom profession '" << c_data.id.toString() << "' but I can't find it!";
                return;
            }
        }
        //edit
        int accepted = cp->show_builder_dialog(m_main_window);
        if(c_data.type == CUSTOM_ICON){
            if(accepted) //if a new icon is started, but cancelled don't save it
                m_custom_prof_icns.insert(cp->prof_id(),cp);
        }else{
            m_custom_professions.insert(cp->get_name(),cp);
        }
        //update other stuff if updated
        if (accepted) {
            m_main_window->load_customizations();
            write_custom_professions();
        }
    }else{
        SuperLabor *sl;
        sl = m_super_labors.take(c_data.id.toString());
        if(!sl){
            LOGW << "tried to edit super labor '" << c_data.id.toString() << "' but I can't find it!";
            return;
        }
        int accepted = sl->show_builder_dialog(m_main_window);
        m_super_labors.insert(sl->get_name(),sl);
        if (accepted) {
            m_main_window->load_customizations();
            write_super_labors();
        }
    }
}

void DwarfTherapist::delete_customization() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    DwarfTherapist::customization_data c_data = build_c_data(a->data());

    if(c_data.type != CUSTOM_SUPER){
        CustomProfession *cp;
        if(c_data.type == CUSTOM_ICON)
            cp = get_custom_prof_icon(c_data.id.toInt());
        else
            cp = get_custom_profession(c_data.id.toString());

        if (!cp) {
            LOGW << "tried to delete custom profession '" << c_data.id.toString() << "' but it wasn't found!";
            return;
        }
        if(c_data.type == CUSTOM_PROF){ //custom profession
            QStringList in_use_by;
            foreach(Dwarf *d, m_main_window->get_model()->get_dwarves()) {
                if (d->custom_profession_name() == c_data.id.toString()) {
                    in_use_by.append(d->nice_name());
                }
            }
            if (in_use_by.size() > 0) {
                QMessageBox *box = new QMessageBox(m_main_window);
                box->setIcon(QMessageBox::Warning);
                box->setWindowTitle(tr("Cannot Remove Profession"));
                box->setText(tr("<b>%1</b> is still in use by: %2 %3. Please change them to"
                                " another profession before deleting this profession!")
                             .arg(capitalize(c_data.id.toString())).arg(in_use_by.size()).arg((in_use_by.size() > 1 ? tr("units") : tr("unit"))));
                box->setDetailedText(tr("Units with this profession:\n\n%1").arg(in_use_by.join("\n")));
                box->exec();
                return;
            }
        }
        del_custom_profession(cp);
    }else{
        SuperLabor *sl = get_super_labor(c_data.id.toString());
        if (!sl) {
            LOGW << "tried to delete super labor '" << c_data.id.toString() << "'' but it wasn't found!";
            return;
        }
        sl->delete_from_disk();
        m_super_labors.remove(c_data.id.toString());
        delete sl;
        m_main_window->load_customizations();
        write_super_labors();
    }
}

void DwarfTherapist::del_custom_profession(CustomProfession *cp){
    cp->delete_from_disk();
    if(cp->prof_id() < 0){
        m_custom_professions.take(cp->get_name());
    }else{
        m_custom_prof_icns.take(cp->prof_id());
    }
    delete cp;
    m_main_window->load_customizations();
    write_custom_professions();
}

//! convenience method
Dwarf *DwarfTherapist::get_dwarf_by_id(int dwarf_id) {
    return m_main_window->get_model()->get_dwarf_by_id(dwarf_id);
}

void DwarfTherapist::load_game_translation_tables(DFInstance *df) {
    LOGI << "Loading language translation tables";
    qDeleteAll(m_language);
    m_language.clear();
    m_generic_words.clear();
    m_dwarf_words.clear();

    VIRTADDR generic_lang_table = df->memory_layout()->global_address("language_vector");
    VIRTADDR translation_vector = df->memory_layout()->global_address("translation_vector");
    VIRTADDR word_table_offset = df->memory_layout()->language_offset("word_table");
    TRACE << "LANGUAGES VECTOR" << hexify(translation_vector);
    TRACE << "GENERIC LANGUAGE VECTOR" << hexify(generic_lang_table);
    TRACE << "WORD TABLE OFFSET" << hexify(word_table_offset);

    df->attach();
    if (generic_lang_table != 0xFFFFFFFF && generic_lang_table != 0) {
        LOGI << "Loading generic strings from" << hex << generic_lang_table;
        QVector<VIRTADDR> generic_words = df->enumerate_vector(generic_lang_table);
        LOGI << "generic words" << generic_words.size();
        foreach(VIRTADDR word_ptr, generic_words) {
            m_generic_words << df->read_string(word_ptr);
            m_language << Word::get_word(df, word_ptr);
        }
    }

    if (translation_vector != 0xFFFFFFFF && translation_vector != 0) {
        QVector<VIRTADDR> languages = df->enumerate_vector(translation_vector);
        uint dwarf_entry = 0;
        foreach(uint lang, languages) {
            QString race_name = df->read_string(lang);
            LOGI << "FOUND LANG ENTRY" << hex << lang << race_name;
            if (race_name == "DWARF")
                dwarf_entry = lang;
        }
        uint dwarf_lang_table = dwarf_entry + word_table_offset;
        LOGI << "Loading dwarf strings from" << hex << dwarf_lang_table;
        QVector<VIRTADDR> dwarf_words = df->enumerate_vector(dwarf_lang_table);
        LOGI << "dwarf words" << dwarf_words.size();

        foreach(VIRTADDR word_ptr, dwarf_words) {
            m_dwarf_words << df->read_string(word_ptr);
        }
    }
    df->detach();
}

void DwarfTherapist::emit_customizations_changed(){
    emit customizations_changed();
    m_main_window->get_view_manager()->redraw_current_tab();
}

void DwarfTherapist::emit_settings_changed(){
    emit settings_changed();
}

void DwarfTherapist::emit_roles_changed(){
    emit roles_changed();
}

void DwarfTherapist::emit_labor_counts_updated(){
    if(m_main_window->get_DFInstance()){
        emit labor_counts_updated();
        if(get_main_window()->get_view_manager())
            get_main_window()->get_view_manager()->redraw_current_tab_headers();
    }
}

void DwarfTherapist::update_specific_header(int id, COLUMN_TYPE type){
    if(m_main_window->get_DFInstance() && get_main_window()->get_view_manager())
        get_main_window()->get_view_manager()->redraw_specific_header(id,type);
}
