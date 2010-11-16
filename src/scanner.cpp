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
#include <QThread>
#include "scanner.h"
#include "dfinstance.h"
#include "gamedatareader.h"
#include "dwarftherapist.h"
#include "scannerthread.h"
#include "defines.h"
#include "selectparentlayoutdialog.h"
#include "layoutcreator.h"

Scanner::Scanner(DFInstance *df, MainWindow *parent)
    : QDialog(parent)
    , m_df(df)
    , m_thread(0)
    , ui(new Ui::ScannerDialog)
    , m_stop_scanning(false)
{
    ui->setupUi(this);
    set_ui_enabled(true);
}

void Scanner::cancel_scan() {
    m_stop_scanning = true;
    m_df->cancel_scan();
}

void Scanner::set_ui_enabled(bool enabled) {
    m_stop_scanning = enabled;
    ui->gb_scan_targets->setEnabled(enabled);
    ui->gb_search->setEnabled(enabled);
    ui->gb_brute_force->setEnabled(enabled);
    ui->gb_progress->setEnabled(!enabled);
    ui->btn_cancel_scan->setEnabled(!enabled);
    ui->lbl_scan_progress->setText(tr("Not Scanning"));
    ui->pb_main->reset();
    ui->pb_sub->reset();
}

void Scanner::report_address(const QString &msg, const quint32 &addr) {
    VIRTADDR corrected_addr = addr - m_df->get_memory_correction();
    QString out = QString("<b>%1\t= <font color=blue>%2</font> "
                          "(uncorrected:%3)\n")
        .arg(msg)
        .arg(hexify(corrected_addr))
        .arg(hexify(addr));
    ui->text_output->append(out);
    LOGD << "ADDRESS FOR:" << msg << hexify(corrected_addr) << "UNCORRECTED:" <<
            hexify(addr);
}

void Scanner::report_offset(const QString &msg, const int &addr) {
    QString out = QString("<b>%1\t= <font color=blue>%2</font></b>\n")
        .arg(msg)
        .arg(hexify(addr));
    ui->text_output->append(out);
    LOGD << "OFFSET FOR:" << msg << hexify(addr);
}

void Scanner::prepare_new_thread(SCANNER_JOB_TYPE type) {
    if (m_thread && m_thread->isRunning()) {
        m_thread->terminate();
        m_thread->wait(3000);
        m_thread->deleteLater();
        m_thread = 0;
    }

    m_thread = new ScannerThread(m_df);
    m_thread->set_job(type);
    connect(m_thread, SIGNAL(main_scan_total_steps(int)),
            ui->pb_main, SLOT(setMaximum(int)));
    connect(m_thread, SIGNAL(main_scan_progress(int)),
            ui->pb_main, SLOT(setValue(int)));
    connect(m_thread, SIGNAL(sub_scan_total_steps(int)),
            ui->pb_sub, SLOT(setMaximum(int)));
    connect(m_thread, SIGNAL(sub_scan_progress(int)),
            ui->pb_sub, SLOT(setValue(int)));
    connect(m_thread, SIGNAL(scan_message(const QString&)),
            ui->lbl_scan_progress, SLOT(setText(const QString&)));
    connect(m_thread, SIGNAL(found_address(const QString&, const quint32&)),
            SLOT(report_address(const QString&, const quint32&)));
    connect(m_thread, SIGNAL(found_offset(const QString&, const int&)),
            SLOT(report_offset(const QString&, const int&)));
}

void Scanner::run_thread_and_wait() {
    if (!m_thread) {
        LOGW << "can't run a thread that was never set up! (m_thread == 0)";
        return;
    }
    m_thread->start();
    while (!m_thread->wait(200)) {
        if (m_stop_scanning || !m_thread->isRunning() || m_thread->isFinished())
            break;
        //ui->text_output->append("waiting on thread...");
        DT->processEvents();
    }
    m_thread->terminate();
    if (m_thread->wait(5000)) {
        delete m_thread;
    } else {
        LOGE << "Scanning thread failed to stop for 5 seconds after killed!";
    }
    m_thread = 0;
}

void Scanner::find_translations_vector() {
    set_ui_enabled(false);
    prepare_new_thread(FIND_TRANSLATIONS_VECTOR);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::find_dwarf_race_index() {
    set_ui_enabled(false);
    prepare_new_thread(FIND_DWARF_RACE_INDEX);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::find_creature_vector() {
    set_ui_enabled(false);
    prepare_new_thread(FIND_CREATURE_VECTOR);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::find_vector_by_length() {
    set_ui_enabled(false);
    uint target_count = ui->sb_vector_entries->value();
    ui->text_output->append(tr("Vectors with %1 entries").arg(target_count));
    prepare_new_thread(FIND_VECTORS_OF_SIZE);
    QByteArray needle = QString("%1").arg(target_count).toAscii();
    m_thread->set_meta(needle);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::find_stone_vector() {
    set_ui_enabled(false);
    prepare_new_thread(FIND_STONE_VECTOR);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::find_metal_vector() {
    set_ui_enabled(false);
    set_ui_enabled(true);
}

void Scanner::find_position_vector() {
    set_ui_enabled(false);
    prepare_new_thread(FIND_POSITION_VECTOR);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::create_memory_layout() {
    set_ui_enabled(false);

    SelectParentLayoutDialog dlg(m_df, this);
    int ret = dlg.exec();

    if(QDialog::Accepted == ret)
    {
        GameDataReader * gdr = GameDataReader::ptr();
        QString out = QString("<b><font color=blue>Make sure that your first dwarf from embark has a nickname of '%1' and a custom profession of '%2'</font></b>\n")
                      .arg(gdr->get_string_for_key("ram_guesser/dwarf_nickname"))
                      .arg(gdr->get_string_for_key("ram_guesser/dwarf_custom_profession"));
        ui->text_output->append(out);
        MemoryLayout * parent = dlg.get_layout();
        LOGD << "Attempting to create layout from " << parent->game_version() << " for version "
                << dlg.get_version_name() << " and filename " << dlg.get_file_name();

        LayoutCreator * creator = new LayoutCreator(m_df, parent, dlg.get_file_name(), dlg.get_version_name());
        m_df->set_memory_layout(parent);

        ScannerJob::m_layout_override_checksum = parent->checksum();

        prepare_new_thread(FIND_DWARF_RACE_INDEX);
        connect(m_thread, SIGNAL(found_address(const QString&, const quint32&)), creator,
                SLOT(report_address(const QString&, const quint32&)));
        run_thread_and_wait();

        prepare_new_thread(FIND_TRANSLATIONS_VECTOR);
        connect(m_thread, SIGNAL(found_address(const QString&, const quint32&)), creator,
                SLOT(report_address(const QString&, const quint32&)));
        run_thread_and_wait();

        prepare_new_thread(FIND_CREATURE_VECTOR);
        connect(m_thread, SIGNAL(found_address(const QString&, const quint32&)), creator,
                SLOT(report_address(const QString&, const quint32&)));
        run_thread_and_wait();

        ScannerJob::m_layout_override_checksum = "";

        LOGD << "Finished reading layouts, writing to disk.";
        if(creator->write_file()) {
            out = QString("<b><font color=green>Finished. Created new file: %1</font></b>\n").arg(dlg.get_file_name());
            ui->text_output->append(out);
            out = QString("<b><font color=red>Please restart DwarfTherapist!</font></b>\n");
            ui->text_output->append(out);

            LOGD << "Finished writing file " << dlg.get_file_name() << " to disk.";
        } else {
            out = QString("<b><font color=red>Unable to write to file, please check your disk!</font></b>\n");
            ui->text_output->append(out);
        }
        delete creator;
    }

    set_ui_enabled(true);
}

void Scanner::find_std_string() {
    set_ui_enabled(false);
    prepare_new_thread(FIND_STD_STRING);
    QByteArray needle = ui->le_null_terminated_string->text().toAscii();
    m_thread->set_meta(needle);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::find_null_terminated_string() {
    set_ui_enabled(false);
    prepare_new_thread(FIND_NULL_TERMINATED_STRING);
    QByteArray needle = ui->le_null_terminated_string->text().toLocal8Bit();
    m_thread->set_meta(needle);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::find_number_or_address() {
    set_ui_enabled(false);
    //re-use the basic bit searcher
    prepare_new_thread(FIND_NULL_TERMINATED_STRING);
    bool ok;
    QByteArray needle = encode(ui->le_find_address->text().
                               toUInt(&ok, ui->rb_hex->isChecked() ? 16 : 10));
    m_thread->set_meta(needle);
    run_thread_and_wait();
    set_ui_enabled(true);
}

void Scanner::brute_force_read() {
    set_ui_enabled(false);
    bool ok; // for base conversions
    VIRTADDR addr = ui->le_address->text().toUInt(&ok, 16);
    if (m_df && m_df->is_ok()) {
        switch(ui->cb_interpret_as_type->currentIndex()) {
        case 0: // std::string
            ui->le_read_output->setText(m_df->read_string(addr));
            break;
        case 1: // null terminated string
            {
                QByteArray str(512, 0);
                m_df->read_raw(addr, 512, str);
                ui->le_read_output->setText(QString(str));
            }
            break;
        case 2: // int
            ui->le_read_output->setText(QString::number(m_df->read_int(addr)));
            break;
        case 3: // uint
            ui->le_read_output->setText(QString::number(m_df->read_addr(addr)));
            break;
        case 4: // short
            ui->le_read_output->setText(QString::number(m_df->read_short(addr)));
            break;
        case 5: // ushort
            ui->le_read_output->setText(QString::number(m_df->read_word(addr)));
            break;
        case 6: // std::vector<void*>
            {
                QVector<uint> addresses = m_df->enumerate_vector(addr);
                ui->text_output->append(QString("Vector at %1 contains %2 "
                                                "entries...").arg(hexify(addr))
                                        .arg(addresses.size()));
                foreach(uint a, addresses) {
                    ui->text_output->append(hexify(a));
                }
            }
            break;
        case 7: // raw
            ui->text_output->append(m_df->pprint(addr, 0xA00));
            break;
        }
    } else {
        LOGE << "Cannot brute-force read. DF Connection is not ok.";

    }
    set_ui_enabled(true);
}

void Scanner::find_narrowing() {
    set_ui_enabled(false);
    uint target_count = ui->le_narrowing_value->text().toInt();
    prepare_new_thread(FIND_NARROWING_VECTORS_OF_SIZE);
    QByteArray needle = QString("%1").arg(target_count).toAscii();
    m_thread->set_meta(needle);

    if(ui->lbl_narrowing_result->text() != "nil") {
        m_thread->set_search_vector(m_narrow);
    }

    m_thread->start();
    while (!m_thread->wait(200)) {
        if (m_stop_scanning || !m_thread->isRunning() || m_thread->isFinished())
            break;
        //ui->text_output->append("waiting on thread...");
        DT->processEvents();
    }
    m_thread->terminate();
    if (!m_thread->wait(5000)) {
        LOGE << "Scanning thread failed to stop for 5 seconds after killed!";
        return;
    }

    QVector<VIRTADDR> * result = (QVector<VIRTADDR> *)m_thread->get_result();
    if(result == NULL) {
        ui->lbl_narrowing_result->setText(tr("0"));
        m_narrow.clear();
    } else {
        ui->lbl_narrowing_result->setText(QString("%1").arg(result->size()));
        m_narrow = *result;
        m_thread->clear_result();
        delete result;
    }

    delete m_thread;
    m_thread = 0;

    set_ui_enabled(true);
}

void Scanner::reset_narrowing() {
    LOGD << "Reset narrowing search";
    m_narrow.clear();
    ui->lbl_narrowing_result->setText(tr("nil"));
    ui->le_narrowing_value->setText("");
}

void Scanner::print_narrowing() {
    if(m_narrow.size() > 200) {
        QString out = QString("<b><font color=red>There are a total of %1 vectors, only printing 200.</font></b>\n")
            .arg(m_narrow.size());

        ui->text_output->append(out);
    }

    int i = 0;
    foreach(uint addr, m_narrow) {
        report_address("vector found at", addr);

        if(i++ >= 200)
            break;
    }
}

void Scanner::find_squad_vector() {
    set_ui_enabled(false);
    prepare_new_thread(FIND_SQUADS_VECTOR);
    run_thread_and_wait();
    set_ui_enabled(true);
}
