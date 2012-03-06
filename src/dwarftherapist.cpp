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
#include <QtGui>

#include "dwarftherapist.h"
#include "mainwindow.h"
#include "optionsmenu.h"
#include "version.h"
#include "customprofession.h"
#include "dwarfmodel.h"
#include "dwarfmodelproxy.h"
#include "dwarf.h"
#include "word.h"
#include "ui_mainwindow.h"
#include "dfinstance.h"
#include "memorylayout.h"
#include "truncatingfilelogger.h"

DwarfTherapist::DwarfTherapist(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_user_settings(0)
    , m_main_window(0)
    , m_options_menu(0)
    , m_reading_settings(false)
    , m_allow_labor_cheats(false)
    , m_log_mgr(0)
{
    setup_logging();
    load_translator();

    TRACE << "Creating settings object";
    m_user_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
    TRACE << "Creating options menu";
    m_options_menu = new OptionsMenu;
    TRACE << "Creating main window";
    m_main_window = new MainWindow;
    m_options_menu->setParent(m_main_window, Qt::Dialog);

    TRACE << "connecting signals";
    connect(m_options_menu, SIGNAL(settings_changed()), SIGNAL(settings_changed())); // the telephone game...
    connect(m_options_menu, SIGNAL(settings_changed()), this, SLOT(read_settings()));
    connect(m_main_window->ui->act_options, SIGNAL(triggered()), m_options_menu, SLOT(exec()));
    connect(m_main_window->ui->act_import_existing_professions, SIGNAL(triggered()), this, SLOT(import_existing_professions()));
    connect(m_main_window->ui->list_custom_professions, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(edit_custom_profession(QListWidgetItem*)));
    connect(m_main_window->ui->act_add_custom_profession, SIGNAL(triggered()), this, SLOT(add_custom_profession()));
    connect(m_main_window->ui->le_filter_text, SIGNAL(textChanged(const QString&)), m_main_window->get_proxy(), SLOT(setFilterFixedString(const QString&)));

    read_settings();

    bool read = m_user_settings->value("options/read_on_startup", true).toBool();
    if (read) {
        QTimer::singleShot(0, m_main_window, SLOT(connect_to_df()));
    }
    m_main_window->show();
}

void DwarfTherapist::setup_logging() {
    QStringList args = arguments();
    bool debug_logging = args.indexOf("-debug") != -1;
    bool trace_logging = args.indexOf("-trace") != -1;

    //TODO REMOVE ME
    // debug logging on by default for the early builds...
    debug_logging = true;

    LOG_LEVEL min_level = LL_INFO;
    if (trace_logging) {
        min_level = LL_TRACE;
    } else if (debug_logging) {
        min_level = LL_DEBUG;
    }

    //setup logging
    m_log_mgr = new LogManager(this);
    TruncatingFileLogger *log = m_log_mgr->add_logger("log/run.log");
    if (log) {
        LogAppender *app = m_log_mgr->add_appender("core", log, LL_TRACE);
        if (app) {
            Version v; // current version
            LOGI << "Dwarf Therapist" << v.to_string() << "starting normally.";
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

void DwarfTherapist::load_translator() {
    TRACE << "loading translations";
    QTranslator translator;
    translator.load("dwarftherapist_en");
    installTranslator(&translator);
    TRACE << "english translation loaded";
}

void DwarfTherapist::read_settings() {
    LOGD << "beginning to read settings";
    m_reading_settings = true; // don't allow writes while we're reading...

    // HACK!
    if (m_user_settings->value("it_feels_like_the_first_time", true).toBool() ||
        !m_user_settings->contains("options/colors/happiness/1")) {
        m_options_menu->write_settings(); //write it out so that we can get default colors loaded
        emit settings_changed(); // this will cause delegates to get the right default colors
        m_user_settings->setValue("it_feels_like_the_first_time", false);
    }

    if (m_user_settings->value("options/show_toolbutton_text", true).toBool()) {
        m_main_window->get_toolbar()->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    } else {
        m_main_window->get_toolbar()->setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

    foreach(CustomProfession *cp, m_custom_professions) {
        cp->deleteLater();
    }
    m_custom_professions.clear();

    m_user_settings->beginGroup("custom_professions");
    {
        QStringList profession_names = m_user_settings->childGroups();
        foreach(QString prof, profession_names) {
            CustomProfession *cp = new CustomProfession(this);
            cp->set_name(prof);
            m_user_settings->beginGroup(prof);
            int size = m_user_settings->beginReadArray("labors");
            for(int i = 0; i < size; ++i) {
                m_user_settings->setArrayIndex(i);
                int labor_id = m_user_settings->childKeys()[0].toInt();
                cp->add_labor(labor_id);
            }
            m_user_settings->endArray();
            m_user_settings->endGroup();
            m_custom_professions << cp;
        }
    }
    m_user_settings->endGroup();

    m_allow_labor_cheats = m_user_settings->value("options/allow_labor_cheats", false).toBool();

    m_reading_settings = false;
    m_main_window->draw_professions();
    LOGD << "finished reading settings";
}

void DwarfTherapist::write_settings() {
    if (m_custom_professions.size() > 0) {
        m_user_settings->beginGroup("custom_professions");
        m_user_settings->remove(""); // clear all of them, so we can re-write

        foreach(CustomProfession *cp, m_custom_professions) {
            m_user_settings->beginGroup(cp->get_name());
            m_user_settings->beginWriteArray("labors");
            int i = 0;
            foreach(int labor_id, cp->get_enabled_labors()) {
                m_user_settings->setArrayIndex(i++);
                m_user_settings->setValue(QString::number(labor_id), true);
            }
            m_user_settings->endArray();
            m_user_settings->endGroup();
        }
        m_user_settings->endGroup();
    }
}


/* PROFESSIONS */
void DwarfTherapist::import_existing_professions() {
    int imported = 0;
    foreach(Dwarf *d, m_main_window->get_model()->get_dwarves()) {
        QString prof = d->custom_profession_name();
        if (prof.isEmpty())
            continue;
        CustomProfession *cp = get_custom_profession(prof);
        if (!cp) { // import it
            cp = new CustomProfession(d, this);
            cp->set_name(prof);
            m_custom_professions << cp;
            imported++;
        }
    }
    m_main_window->draw_professions();
    QMessageBox::information(m_main_window, tr("Import Successful"),
        tr("Imported %n custom profession(s)", "", imported));
}

CustomProfession *DwarfTherapist::get_custom_profession(QString name) {
    CustomProfession *retval = 0;
    foreach(CustomProfession *cp, m_custom_professions) {
        if (cp && cp->get_name() == name) {
            retval = cp;
            break;
        }
    }
    return retval;
}

void DwarfTherapist::add_custom_profession(CustomProfession *cp) {
    m_custom_professions << cp;
    m_main_window->draw_professions();
    write_settings();
}

void DwarfTherapist::add_custom_profession() {
    Dwarf *d = 0;
    custom_profession_from_dwarf(d);
}


//! from CP context menu's "Edit..."
void DwarfTherapist::edit_custom_profession() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString cp_name = a->data().toString();
    CustomProfession *cp = get_custom_profession(cp_name);
    if (!cp) {
        LOGW << "tried to edit custom profession '" << cp_name << "' but I can't find it!";
        return;
    }
    int accepted = cp->show_builder_dialog(m_main_window);
    if (accepted) {
        m_main_window->draw_professions();
        write_settings();
    }
}

//! from double-clicking a profession
void DwarfTherapist::edit_custom_profession(QListWidgetItem *i) {
    QString name = i->text();
    CustomProfession *cp = get_custom_profession(name);
    if (!cp) {
        LOGW << "tried to edit custom profession '" << name << "' but I can't find it!";
        return;
    }
    int accepted = cp->show_builder_dialog(m_main_window);
    if (accepted) {
        m_main_window->draw_professions();
        write_settings();
    }
}

void DwarfTherapist::delete_custom_profession() {
    QAction *a = qobject_cast<QAction*>(QObject::sender());
    QString cp_name = a->data().toString();
    CustomProfession *cp = get_custom_profession(cp_name);
    if (!cp) {
        LOGW << "tried to delete custom profession '" << cp_name << "' but I can't find it!";
        return;
    }

    QList<Dwarf*> blockers;
    foreach(Dwarf *d, m_main_window->get_model()->get_dwarves()) {
        if (d->custom_profession_name() == cp_name) {
            blockers << d;
        }
    }
    if (blockers.size() > 0) {
        QMessageBox *box = new QMessageBox(m_main_window);
        box->setIcon(QMessageBox::Warning);
        box->setWindowTitle(tr("Cannot Remove Profession"));
        box->setText(tr("The following %1 dwarf(s) is(are) still using <b>%2</b>. Please change them to"
            " another profession before deleting this profession!").arg(blockers.size()).arg(cp_name));
        QString msg = tr("Dwarfs with this profession:\n\n");
        foreach(Dwarf *d, blockers) {
            msg += d->nice_name() + "\n";
        }
        box->setDetailedText(msg);
        box->exec();
    } else {
        cp->delete_from_disk();
        m_custom_professions.remove(m_custom_professions.indexOf(cp));
    }
    m_main_window->draw_professions();
    write_settings();
}

int DwarfTherapist::custom_profession_from_dwarf(Dwarf *d) {
    CustomProfession *cp = new CustomProfession(d, this);
    int accepted = cp->show_builder_dialog(m_main_window);
    if (accepted) {
        m_custom_professions << cp;
        m_main_window->draw_professions();
        write_settings();
    }
    return accepted;
}

//! convenience method
Dwarf *DwarfTherapist::get_dwarf_by_id(int dwarf_id) {
    return m_main_window->get_model()->get_dwarf_by_id(dwarf_id);
}

void DwarfTherapist::load_game_translation_tables(DFInstance *df) {
    LOGD << "Loading language translation tables";
    m_language.clear();
    m_generic_words.clear();
    m_dwarf_words.clear();

    uint generic_lang_table = df->memory_layout()->address("language_vector") + df->get_memory_correction();
    uint translation_vector = df->memory_layout()->address("translation_vector") + df->get_memory_correction();
    uint word_table_offset = df->memory_layout()->offset("word_table");
    TRACE << "LANGUAGES VECTOR" << hex << translation_vector;
    TRACE << "GENERIC LANGUAGE VECTOR" << hex << generic_lang_table;
    TRACE << "WORD TABLE OFFSET" << hex << word_table_offset;

    df->attach();
    if (generic_lang_table != 0xFFFFFFFF && generic_lang_table != 0) {
        LOGD << "Loading generic strings from" << hex << generic_lang_table;
        QVector<uint> generic_words = df->enumerate_vector(generic_lang_table);
        LOGD << "generic words" << generic_words.size();
        foreach(uint word_ptr, generic_words) {
            m_generic_words << df->read_string(word_ptr);
            m_language << Word::get_word(df, word_ptr);
        }
    }

    if (translation_vector != 0xFFFFFFFF && translation_vector != 0) {
        QVector<uint> languages = df->enumerate_vector(translation_vector);
        uint dwarf_entry = 0;
        foreach(uint lang, languages) {
            QString race_name = df->read_string(lang);
            LOGD << "FOUND LANG ENTRY" << hex << lang << race_name;
            if (race_name == "DWARF")
                dwarf_entry = lang;
        }
        uint dwarf_lang_table = dwarf_entry + word_table_offset - df->VECTOR_POINTER_OFFSET;
        LOGD << "Loading dwarf strings from" << hex << dwarf_lang_table;
        QVector<uint> dwarf_words = df->enumerate_vector(dwarf_lang_table);
        LOGD << "dwarf words" << dwarf_words.size();

        foreach(uint word_ptr, dwarf_words) {
            m_dwarf_words << df->read_string(word_ptr);
        }
    }
    df->detach();
}
