#ifndef DWARF_H
#define DWARF_H

#include <QObject>
class DFInstance;

class Dwarf : public QObject
{
	Q_OBJECT
	Dwarf(DFInstance *df, int address, QObject *parent=0); //private, use the static get_dwarf() method

public:
	static Dwarf* get_dwarf(DFInstance *df, int address);
	~Dwarf();

	QString nice_name();
	QString to_string();

private:
	DFInstance *m_df;
	int m_address;
	int m_race_id;
	QString read_last_name();

	QString m_first_name;
	QString m_last_name;
	QString m_nick_name;
	QString m_custom_profession;
	int m_id;
};

#endif // DWARF_H
