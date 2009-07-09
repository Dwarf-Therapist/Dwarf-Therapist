#ifndef DFINSTANCE_H
#define DFINSTANCE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <QObject>

class DFInstance : public QObject {
private:
	Q_OBJECT
    DFInstance(DWORD pid, HWND hwnd, QObject *parent=0);
public:
    ~DFInstance();
    static DFInstance* find_running_copy(QObject *parent=0);
    
	QVector<int> enumerate_vector(int address);

    char read_char(int start_address, uint &bytes_read);
    int read_int32(int start_address, uint &bytes_read);
    int read_raw(int start_address, int bytes, char *buffer);
	QVector<int> scan_mem_find_all(QByteArray &needle, int start_address, int end_address);
    int scan_mem(QByteArray &needle, int start_address, int end_address, bool &ok);
    QString read_string(int start_address);
    

    QString get_base_address();
    uint find_creature_vector();

    public slots:
        void cancel_scan() {
            m_stop_scan = true;
        }

private:
    int calculate_checksum();
    QByteArray encode_int(uint addr);

    DWORD m_pid;
    HWND m_hwnd;
    HANDLE m_proc;
    uint m_base_addr;
    uint m_memory_size;
    int m_memory_correction;
    bool m_stop_scan;

    static const int STRING_BUFFER_OFFSET = 4;
    static const int STRING_LENGTH_OFFSET = 20;
    static const int STRING_CAP_OFFSET = 24;

signals:
    void scan_total_steps(int steps);
    void scan_progress(int step);

};

#endif // DFINSTANCE_H
