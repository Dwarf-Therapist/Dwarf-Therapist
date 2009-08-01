#include <QtGui>
#include <QxtLogger>

#include "dwarftherapist.h"
#include "truncatingfilelogger.h"
#include "mainwindow.h"
#include "optionsmenu.h"
#include "version.h"

DwarfTherapist::DwarfTherapist(int &argc, char **argv) 
	: QApplication(argc, argv)
	, m_user_settings(0)
	, m_main_window(0)
	, m_options_menu(0)
	, m_reading_user_settings(false)
{
	setup_logging();
	load_translator();

	m_user_settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, COMPANY, PRODUCT, this);
	m_main_window = new MainWindow;
	m_options_menu = new OptionsMenu(m_main_window);

	connect(m_options_menu, SIGNAL(settings_changed()), SIGNAL(settings_changed())); // the telephone game...


	QAction *act_options = m_main_window->findChild<QAction *>("act_options");
	connect(act_options, SIGNAL(triggered()), m_options_menu, SLOT(show()));

	m_main_window->show();
}

void DwarfTherapist::setup_logging() {
	QStringList args = arguments();
	bool debug_logging = args.indexOf("-debug") != -1;
	bool trace_logging = args.indexOf("-trace") != -1;

	//TODO REMOVE ME
	// debug logging on by default for the early builds...
	debug_logging = true;

	//setup logging
	TruncatingFileLoggerEngine *engine = new TruncatingFileLoggerEngine("log/run.log");
	qxtLog->addLoggerEngine("main", engine);
	QxtLogger::getInstance()->installAsMessageHandler();
	qxtLog->setMinimumLevel(QxtLogger::InfoLevel);

	Version v; // current version
	LOG->info("Dwarf Therapist", v.to_string(), "starting normally.");
	if (debug_logging && !trace_logging) {
		qxtLog->setMinimumLevel(QxtLogger::DebugLevel);
		LOG->info("MINIMUM LOG LEVEL SET TO: DEBUG");
	} else if (trace_logging) {
		qxtLog->setMinimumLevel(QxtLogger::TraceLevel);
		LOG->info("MINIMUM LOG LEVEL SET TO: TRACE");
	} else {
		LOG->info("MINIMUM LOG LEVEL SET TO: INFO");
	}
}

void DwarfTherapist::load_translator() {
	LOGD << "loading translations";
	QTranslator translator;
	translator.load("dwarftherapist_en");
	installTranslator(&translator);
}