#ifndef DFINSTANCE_H
#define DFINSTANCE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <QObject>
#include <QString>

#include "dwarf.h"

class DFInstance : public QObject {
private:
	Q_OBJECT
    DFInstance(DWORD pid, HWND hwnd, QObject *parent=0);
public:
    ~DFInstance();
    typedef QVector<int> AddressVector;
    //typedef AddressVector::iterator AddressVectorIter;

    // factory ctor
    static DFInstance* find_running_copy(QObject *parent=0);

    // accessors
	int get_memory_correction() {return m_memory_correction;}
	int get_base_address() {return m_base_addr;}
	bool is_ok(){return m_is_ok;}
    
    // brute force memory scanning methods
    QVector<int> enumerate_vector(int address);
    char read_char(int start_address, uint &bytes_read);
    short read_short(int start_address, uint &bytes_read);
	ushort read_ushort(int start_address, uint &bytes_read);
    int read_int32(int start_address, uint &bytes_read);
    int read_raw(int start_address, int bytes, void *buffer);
    int scan_mem(QByteArray &needle, int start_address, int end_address, bool &ok);
    QVector<int> scan_mem_find_all(QByteArray &needle, int start_address, int end_address);
    QString read_string(int start_address);
    
    // Mapping methods
    int find_language_vector();
    int find_translation_vector();
    int find_creature_vector();

    // Methods for when we know how the data is layed out
	QVector<Dwarf*> DFInstance::load_dwarves();

	// Writing
	int write_raw(int start_address, int bytes, void *buffer);
	

    public slots:
        // if a menu cancels our scan, we need to know how to stop
        void cancel_scan() {m_stop_scan = true;}

private:
    // handy util methods
    int calculate_checksum();
    QVector<QVector<int> > DFInstance::cross_product(QVector<QVector<int> > addresses, int index);

    DWORD m_pid;
    HWND m_hwnd;
    HANDLE m_proc;
    uint m_base_addr;
    uint m_memory_size;
    int m_memory_correction;
    bool m_stop_scan; // flag that gets set to stop scan loops
	bool m_is_ok;

    // these should probably not be constants but I have no idea how to find the values at runtime
    static const int STRING_BUFFER_OFFSET = 4;
    static const int STRING_LENGTH_OFFSET = 20;
    static const int STRING_CAP_OFFSET = 24;

signals:
    // methods for sending progress information to QWidgets
    void scan_total_steps(int steps);
    void scan_progress(int step);
    void scan_message(const QString &message);

};

#endif // DFINSTANCE_H
