#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtDebug>
#include <QTableWidgetItem>
#include <QProgressDialog>
#include <QTextCodec>

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
    ,m_df(0)
{
    ui->setupUi(this);
    set_interface_enabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connect_to_df() {
    if (m_df) {
        delete m_df;
        set_interface_enabled(false);
        m_df = 0;
    }
    m_df = DFInstance::find_running_copy(this);
    if (!m_df) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Unable to locate a running copy of Dwarf "
                                "Fortress, are you sure it's running?"));
        return;
    }
    this->statusBar()->showMessage(QString("Connected to Dwarf Fortress BASE ADDR: %1").arg(m_df->get_base_address()));
    set_interface_enabled(true);


    /*QProgressDialog *pd = new QProgressDialog(tr("Scanning for Creature Vector"), tr("Cancel"), 0, 1, this);
    connect(m_df, SIGNAL(scan_total_steps(int)), pd, SLOT(setMaximum(int)));
    connect(m_df, SIGNAL(scan_progress(int)), pd, SLOT(setValue(int)));
    connect(pd, SIGNAL(canceled()), m_df, SLOT(cancel_scan()));
    pd->show();

    uint creature_addr = m_df->find_creature_vector();
	*/
}

void MainWindow::read_memory() {
    bool ok;
    int addr = ui->edit_s_address->text().toInt(&ok, 16);

	uint bytes_read = 0;
    int val = m_df->read_int32(addr, bytes_read);
    statusBar()->showMessage(QString("VALUE: 0x%1").arg(val, 8,16, QChar('0')));

    //ui->text->appendPlainText(QString("%1 0x%2").arg(ui->edit_address->text()).arg(val, 0, 16));
    /*
    int bytes = ui->spinner_bytes->value();
    unsigned int bytes_read = 0;
    bool ok;
    qulonglong addr = ui->edit_address->text().toULongLong(&ok, 16);

    QString data = m_df->read_memory(addr, bytes, bytes_read);
    qDebug() << "read " << bytes_read << " bytes";
    ui->text->appendPlainText(QString("%1 [%2]").arg(addr).arg(data));
    ui->text->appendPlainText("------------------------------------------------------------------------------");
    */
}

/*
Basic BaseAddress: 0x290000 (0x400000)
Basic CheckSum: 0x49c82d3f (0x49df2d3f)
Address LanguageVector: 0x13f15c8 (0x15615c8)
Offset Translation.WordTable: 0x58 (0x170058)
Address TranslationsVector: 0x13f15f8 (0x15615f8)
Address CreatureVector: 0x13ab3cc (0x151b3cc)
Offset Creature.CustomProfession: 0x6c (0x17006c)
Offset Creature.Labors: 0x544 (0x170544)
Address DwarvenRaceIndex: 0x11ba850 (0x132a850)
*/

void MainWindow::search_dump() {
	bool ok;
	uint start_addr = ui->edit_s_address->text().toUInt(&ok, 16);
	uint end_addr = ui->edit_e_address->text().toUInt(&ok, 16);
	uint len = end_addr - start_addr;

	if (end_addr <= start_addr) {
		QMessageBox::warning(this, "Bad Addresses", "end address must be higher than the start address!");
		return;
	}

	QString word = ui->edit_search->text();
	QProgressDialog *pd = new QProgressDialog(QString("Searching RAM for %1").arg(word), "Cancel", 0, len, this);
	pd->show();


	int step = 0x1000;
	char *buffer = new char[step];
	uint ptr = start_addr;

	while (ptr < end_addr) {
		int bytes_read = m_df->read_raw(ptr, step, buffer);
		QByteArray needle = word.toAscii();
		QByteArray haystack = QByteArray::fromRawData((char*)buffer, bytes_read);
		int idx = haystack.indexOf(needle, 0);
		if (idx != -1) {
			// NAIIIILED IT
			qDebug() << "FOUND" << word << "at" << hex << ptr + idx;
			pd->cancel();
			return;
		}
		/*
		QTextCodec *codec = QTextCodec::codecForName("IBM 850");
		QString chnk = codec->toUnicode((char*)buffer, bytes_read);
		qDebug() << "CHUNK(((" << chnk << ")))CHUNK\n\n";
		if (chnk.contains(word, Qt::CaseInsensitive)) {
			qDebug() << "FOUND" << word << "at" << hex << chnk.indexOf(word) + ptr;
			pd->cancel();
			return;
		}
		*/
		ptr += step;
		qApp->processEvents();
		if (pd->wasCanceled()) {
			pd->hide();
			break;
		}
		pd->setValue(ptr - start_addr);
	}
}

/*
void MainWindow::search_dump() {
    bool ok;
    uint start_addr = ui->edit_s_address->text().toUInt(&ok, 16);
    uint end_addr = ui->edit_e_address->text().toUInt(&ok, 16);
    uint len = end_addr - start_addr;

    if (end_addr <= start_addr) {
        QMessageBox::warning(this, "Bad Addresses", "end address must be higher than the start address!");
        return;
    }

    QProgressDialog *pd = new QProgressDialog("Dumping RAM", "Cancel", 0, len, this);
    pd->show();

    QString last_word;
    QString word = ui->edit_search->text();
    for (uint i = start_addr; i <= end_addr; i += 16) {
        //wchar_t buffer[word.length()];
        char buffer[16];
        if (m_df->read_string(i, 16, buffer)) {
            //QString str = QString::fromWCharArray(buffer, word.length());
            //QString str = QString::fromAscii(buffer, word.length());
            QTextCodec *codec = QTextCodec::codecForName("IBM 850");
            QString str = last_word + codec->toUnicode(buffer);
            if (str.contains(word, Qt::CaseInsensitive)
                || str.startsWith(word, Qt::CaseInsensitive)) {
                qDebug() << "found " << word << " at " << QString("%1").arg(i, 0, 16);
                pd->cancel();
                return;
            } else {
                //qDebug() << "--" << str << "--";
            }
            last_word = str;
        }

        qApp->processEvents();
        if (pd->wasCanceled()) {
            pd->hide();
            break;
        }
        pd->setValue(i-start_addr);
    }
}
*/

void MainWindow::dump_mem() {
    bool ok;
    uint start_addr = ui->edit_s_address->text().toUInt(&ok, 16);
    uint end_addr = ui->edit_e_address->text().toUInt(&ok, 16);
    uint len = end_addr - start_addr;

    int rows = len / 16;
    if (len % 16) {
        rows++;
    }

    if (rows > 1000) {
        QMessageBox::warning(this, "Range too big!", "Please select start and end addresses within 16K of eachother");
        return;
    }

    int bytes = 16;
    ui->tbl_hex->scrollToTop();
    ui->tbl_hex->verticalHeader()->show();

    QProgressDialog *pd = new QProgressDialog("Dumping RAM", "Cancel", 0, rows, this);
    pd->show();

    ui->tbl_hex->clearContents();

    for (int i = 0; i < rows; ++i) {
        QString ascii = "";
        QTableWidgetItem *addr_header = new QTableWidgetItem(QString("0x%1").arg(start_addr + (i*16), 0, 16));
        addr_header->setFont(QFont("Courier New", 8, QFont::Bold));
        ui->tbl_hex->setVerticalHeaderItem(i, addr_header);
        for (int j = 0; j < bytes; ++j) {
            uint bytes_read = 0;
            char val = m_df->read_char(start_addr + (i * 16) + j, bytes_read);
            if (val > 31 && val < 128) {
                ascii += QChar(val);
            } else {
                ascii += ".";
            }
            QTableWidgetItem *item = 0;
            const QChar fill('0');
            if (ok) {
                item = new QTableWidgetItem(QString("%1").arg(val, 2, 16, fill).toUpper());
            } else {
                item = new QTableWidgetItem("??");
            }
            ui->tbl_hex->setItem(i, j, item);
        }
        ui->tbl_hex->setItem(i, 16, new QTableWidgetItem(ascii));

        qApp->processEvents();
        if (pd->wasCanceled()) {
            break;
        }
        pd->setValue(i+1);
    }
}


void MainWindow::set_interface_enabled(bool enabled) {
    ui->btn_read->setEnabled(enabled);
    ui->btn_dump->setEnabled(enabled);
    ui->edit_s_address->setEnabled(enabled);
    ui->edit_e_address->setEnabled(enabled);
    //ui->spinner_bytes->setEnabled(enabled);
}
