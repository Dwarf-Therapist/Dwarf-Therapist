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
#ifndef DFINSTANCE_H
#define DFINSTANCE_H

#include <QtGui>
#include "utils.h"

class Dwarf;
class MemoryLayout;
struct MemorySegment;

class DFInstance : public QObject {
    Q_OBJECT
public:
    DFInstance(QObject *parent=0);
    virtual ~DFInstance();

    // factory ctor
    virtual bool find_running_copy() = 0;

    // accessors
    uint get_heap_start_address() {return m_heap_start_address;}
    uint get_memory_correction() {return m_memory_correction;}
    uint get_base_address() {return m_base_addr;}
    bool is_ok(){return m_is_ok;}

    // brute force memory scanning methods
    bool is_valid_address(const uint &addr);
    bool looks_like_vector_of_pointers(const uint &addr);

    // revamped memory reading
    virtual int read_raw(const VIRTADDR &addr, int bytes, QByteArray &buf) = 0;
    virtual BYTE read_byte(const VIRTADDR &addr);
    virtual WORD read_word(const VIRTADDR &addr);
    virtual DWORD read_dword(const VIRTADDR &addr);
    virtual qint16 read_short(const VIRTADDR &addr);
    virtual qint32 read_int(const VIRTADDR &addr);

    // memory reading
    virtual QVector<uint> enumerate_vector(const uint &addr) = 0;
    virtual QString read_string(const VIRTADDR &addr) = 0;

    QVector<uint> scan_mem(const QByteArray &needle);
    QByteArray get_data(const uint &addr, const uint &size);
    QString pprint(const uint &addr, const uint &size);
    QString pprint(const QByteArray &ba, const uint &start_addr=0);

    // Mapping methods
    QVector<uint> find_vectors_in_range(const uint &max_entries,
                                        const uint &start_address,
                                        const uint &range_length);
    QVector<uint> find_vectors(int num_entries, int fuzz=0, int entry_size=4);

    // Methods for when we know how the data is layed out
    MemoryLayout *memory_layout() {return m_layout;}
    QVector<Dwarf*> load_dwarves();

    // Writing
    virtual uint write_raw(const uint &addr, const uint &bytes, void *buffer) = 0;
    virtual uint write_string(const uint &addr, const QString &str) = 0;
    virtual uint write_int(const uint &addr, const int &val) = 0;

    bool is_attached() {return m_attach_count > 0;}
    virtual bool attach() = 0;
    virtual bool detach() = 0;

    // Windows string offsets
#ifdef Q_WS_WIN
    static const int STRING_BUFFER_OFFSET = 4;
    static const int STRING_LENGTH_OFFSET = 20;
    static const int STRING_CAP_OFFSET = 24;
    static const int VECTOR_POINTER_OFFSET = 4;
#endif
#ifdef Q_WS_X11
    static const int STRING_BUFFER_OFFSET = 0;
    static const int VECTOR_POINTER_OFFSET = 0;
#endif
#ifdef Q_WS_MAC
    static const int STRING_BUFFER_OFFSET = 0;
    static const int VECTOR_POINTER_OFFSET = 0;
#endif

    public slots:
        // if a menu cancels our scan, we need to know how to stop
        void cancel_scan() {m_stop_scan = true;}

protected:
    // handy util methods
    virtual uint calculate_checksum() = 0;

    int m_pid;
    uint m_base_addr;
    uint m_memory_correction;
    uint m_lowest_address;
    uint m_highest_address;
    uint m_heap_start_address;
    bool m_stop_scan; // flag that gets set to stop scan loops
    bool m_is_ok;
    int m_bytes_scanned;
    MemoryLayout *m_layout;
    QVector<MemorySegment*> m_regions;
    int m_attach_count;
    QTimer *m_heartbeat_timer;
    QTimer *m_memory_remap_timer;
    QTimer *m_scan_speed_timer;

    /*! this hash will hold a map of all loaded and valid memory layouts found
        on disk, the key is a QString of the checksum since other OSs will use
        an MD5 of the binary instead of a PE timestamp */
    QHash<QString, MemoryLayout*> m_memory_layouts; // checksum->layout
    MemoryLayout *get_memory_layout(QString checksum);

    private slots:
        void heartbeat();
        void calculate_scan_rate();
        virtual void map_virtual_memory() = 0;

signals:
    // methods for sending progress information to QWidgets
    void scan_total_steps(int steps);
    void scan_progress(int step);
    void scan_message(const QString &message);
    void connection_interrupted();
    void progress_message(const QString &message);
    void progress_range(int min, int max);
    void progress_value(int value);

};

#endif // DFINSTANCE_H
