#include <QtCore>
#include <QtDebug>
#include <sys/ptrace.h>
#include <errno.h>
#include <wait.h>
#include <new>

static inline QByteArray encode_int(int num) {
    char *bytes;
    bytes = (char*)&num;
    QByteArray arr(bytes, sizeof(int));
    return arr;
}
static inline QByteArray encode(uint num) {
    char *bytes;
    bytes = (char*)&num;
    QByteArray arr(bytes, sizeof(uint));
    return arr;
}
static inline QByteArray encode(const short &num) {
    char *bytes;
    bytes = (char*)&num;
    QByteArray arr(bytes, sizeof(short));
    return arr;
}

struct MemorySegment {
    MemorySegment(const QString &_name, const uint &_start_addr, const uint &_end_addr) 
        : name(_name)
        , start_addr(_start_addr)
        , end_addr(_end_addr)
        , is_heap(false)
    {
        if (name.contains("[heap]"))
            is_heap = true;
        size = end_addr - start_addr;
    }
    QString to_string() {
        return QString("0x%1-0x%2 (%L3 bytes) %4 HEAP: %5")
            .arg(start_addr, 8, 16, QChar('0'))
            .arg(end_addr, 8, 16, QChar('0'))
            .arg(size)
            .arg(name)
            .arg(is_heap);
    }
    uint size;
    QString name;
    uint start_addr;
    uint end_addr;
    bool is_heap;
};

static inline QByteArray encode_skillpattern(const short &skill, const int &exp, const short &rating) {
    QByteArray bytes;
    bytes.reserve(8);
    bytes.append(encode(skill));
    bytes.append(encode(rating));
    bytes.append(encode_int(exp));
    return bytes;
}

class MemoryReader : public QCoreApplication {
    int m_pid;
    uint m_base_addr;
    int m_memory_correction;
    uint m_heap_start;
    
    QVector<MemorySegment*> m_regions;
public:
    MemoryReader(int &argc, char** argv)
        : QCoreApplication(argc, argv)
        , m_pid(0)
        , m_base_addr(0)
        , m_memory_correction(0)
    {
        QStringList args = arguments();
        if (args.size() != 2) {
            qCritical() << "USAGE: ./MemoryReader <PID>";
        } else {
            m_pid = args[1].toInt();
        }
        if (m_pid) {
            scan();
            if (m_base_addr != 0) {
                // working...
                //uint dwarf_race_index = find_dwarf_race_index();
                //qDebug() << "DWARF RACE INDEX FOUND" << hex << dwarf_race_index;
                find_translation_vector();
                
                //uint creature_vector_addr = find_creature_vector();
                //qDebug() << "CREATURE VECTOR FOUND" << hex << creature_vector_addr;
            }
        }
    }

    MemorySegment *parent_segment(const uint &addr) {
        foreach(MemorySegment *seg, m_regions) {
            if (seg->start_addr <= addr && seg->end_addr >= addr)
                return seg;
        }
        return 0;
    }
    
    bool is_valid_address(const uint &addr) {
        bool valid = false;
        foreach(MemorySegment *seg, m_regions) {
            if (addr >= seg->start_addr && addr <= seg->end_addr) {
                valid = true;
                break;
            }
        }
        return valid;
    }

    QVector<uint> enumerate_vector(uint addr) {
        QVector<uint> addrs;
        uint start = read_uint(addr);
        uint end = read_uint(addr + 4 );
        uint bytes = end - start;
        uint entries = bytes / 4;
        uint tmp_addr = 0;

        Q_ASSERT_X(start > 0, "enumerate_vector", "start pointer must be larger than 0");
        Q_ASSERT_X(end > 0, "enumerate_vector", "End must be larger than start!");
        Q_ASSERT_X(start % 4 == 0, "enumerate_vector", "Start must be divisible by 4");
        Q_ASSERT_X(end % 4 == 0, "enumerate_vector", "End must be divisible by 4");
        Q_ASSERT_X(end > start, "enumerate_vector", "End must be larger than start!");
        Q_ASSERT_X((end - start) % 4 == 0, "enumerate_vector", "end - start must be divisible by 4");

        char *stuff = new (std::nothrow) char[bytes];
        if (stuff == 0) {
            qWarning() << "Unable to allocate char array of size" << bytes;
            return addrs;
        }
        uint bytes_read = read_raw(start, bytes, stuff);
        if (bytes_read != bytes) {
            qWarning() << "Tried to read" << bytes << "bytes but only got" << bytes_read;
            return addrs;
        }
        for(uint i = 0; i < bytes; i += 4) {
            memcpy(&tmp_addr, stuff + i, 4);
            if (is_valid_address(tmp_addr)) {
                addrs << tmp_addr;
            } else {
                //qWarning() << "bad addr in vector:" << hex << tmp_addr;
            }
        }
        delete[] stuff;
        //qDebug() << "VECTOR at" << hex << addr << "start:" << start << "end:" << end << "(" << dec << entries << "entries) valid entries" << addrs.size();
        return addrs;
    }

    //! Find all vectors of pointers that have 'num_entries' entries
    QVector<uint> hulk_smash(uint num_entries, uint entry_size=4) {
        QVector<uint> vectors;
        uint target_bytes = num_entries * entry_size;

        /*!
            glibc vectors of pointers look like this
            
            offset in bytes: data
            ------------------------------------
            0: ptr to start of data
            4: ptr to end of data
            8: allocator end (we don't care
        */

        uint int1 = 0; // holds the start val
        uint int2 = 0; // holds the end val 


        // iterate over all known memory segments
        foreach(MemorySegment *seg, m_regions) {
            //qDebug() << "SCANNING REGION" << hex << seg->start_addr << "-" << seg->end_addr << "BYTES:" << dec << seg->size;

            // this buffer will hold the entire memory segment (may need to toned down a bit)
            uchar *buffer = new uchar[seg->size];
            if (!buffer) {
                qCritical() << "unable to allocate char buffer of" << seg->size << "bytes!";
                continue;
            }
            memset(buffer, 0, seg->size); // 0 out the buffer

            // this may read multiple times to populate the entire region in our buffer
            uint bytes_read = read_raw(seg->start_addr, seg->size, buffer);
            if (bytes_read < seg->size) {
                qWarning() << "tried to read" << seg->size << "bytes starting at" << hex << seg->start_addr << "but only got" << dec << bytes_read;
                continue;
            }

            /* we now have this entire memory segment inside buffer. So lets step through it looking for things that look like vectors of pointers.
            we read a uint into int1 and 4 bytes later we read another uint into int2. If int1 is a vector head, then int2 will be larger than int2, 
            evenly divisible by 4, and the difference between the two (divided by four) will tell us how many pointers are in this array. We can also
            check to make sure int1 and int2 reside in valid memory regions. If all of this adds up, then odds are pretty good we've found a vector */
            for (uint i = 0; i < seg->size; i += 4) {
                memcpy(&int1, buffer + i, 4);
                memcpy(&int2, buffer + i + 4, 4);
                if (int1 % 4 == 0 && 
                    int2 % 4 == 0 && 
                    int1 < int2 &&
                    is_valid_address(int1) && 
                    is_valid_address(int2) && 
                    (int2 - int1) == target_bytes)
                {
                    QVector<uint> addrs = enumerate_vector(seg->start_addr + i);
                    if (addrs.size() == num_entries) {
                        //qDebug() << "++++++++++++++++++++++ VECTOR HAS" << addrs.size() << "actual entries!";
                        vectors << seg->start_addr + i;
                        //foreach(uint a, addrs) {
                        //    QString obj_str = read_str2(a);
                        //    qDebug() << "\t" << hex << a << "member str:" << obj_str;
                        //}
                    }
                }
            }
            delete[] buffer;
        }
        return vectors;
    }
    
    void find_translation_vector() {
        // First find all vectors that have the correct number of entries to be lang tables
        int num_words = 2107; // how many words per language
        int total_langs = 4;
        QString first_lang_word = "ABBEY"; // first generic (non-racial) word
        QString first_dwarf_word = "kulet"; // first dwarven word
        uint dwarf_lang_table = 0;

        foreach(uint addr, hulk_smash(num_words)) {
            foreach(uint vec_addr, enumerate_vector(addr)) {
                QString first_entry = read_str2(vec_addr);
                //qDebug() << "+++++ FOUND VECTOR" << hex << addr << "FIRST ENTRY:" << first_entry;
                if (first_entry == first_lang_word) {
                    qDebug() << "FOUND LANGUAGE TABLE" << hex << addr;
                } else if (first_entry == first_dwarf_word) {
                    qDebug() << "FOUND DWARF TABLE" << hex << addr;
                    dwarf_lang_table = addr;
                } else {
                    break;
                    //qDebug() << "FOUND ??? TABLE" << hex << addr << "CORRECTED" << addr - m_heap_start;
                }
                break;
            }
        }

        // A translation entry is basically the race name (i.e. "DWARF") followed by a vector of words shorty afterwards.
        // So we find langs like so...
        // 1) look for a null terminated string "DWARF\0"
        // 2) find a pointer to that buffer (this is the std::string object)
        // 3) find a pointer to the std::string (the translation object)
        // 4) find a pointer to the translation object (an entry in the translations vector)

        QVector<uint> translations_vectors;
        foreach(uint str_buf, scan_mem("DWARF\0")) { // std::string internal buffer
            foreach(uint str, scan_mem(encode(str_buf))) { // std::string
                if (read_str(str) != "DWARF") // can match "DWARF_LIASON" and the like so ignore those...
                    continue;
                foreach(uint str_ptr, scan_mem(encode(str))) { // translation table name attribute?
                    foreach(uint str_ptr_ptr, scan_mem(encode(str_ptr))) { // entry in translations vector
                        translations_vectors << str_ptr_ptr;
                        qDebug() << "Found possible translations vector" << hex << str_ptr_ptr;
                    }
                }
            }
        }
        //qDebug() << "Verifying possible vectors";
        foreach(uint vec, translations_vectors) {
            //qDebug() << "Verifying" << hex << vec;
            QVector<uint> langs = enumerate_vector(vec);
            if (langs.size() == total_langs) {
                //qDebug() << "VECTOR COUNT IS VALID";
                if (read_str(langs.at(0)) == "DWARF") {
                    //qDebug() << "FIRST ENTRY IS FOR DWARF!";
                    qDebug() << "+++ VERIFIED +++";
                    qDebug() << "\tTRANSLATIONS VECTOR AT" << hex << vec;
                    qDebug() << "\tDWARF TRANS OBJECT" << hex << langs.at(0) << "WORD TABLE" << dwarf_lang_table;
                    qDebug() << "\tOFFSET FROM WORD TABLE" << hex << (int)(dwarf_lang_table - langs.at(0));
                    break;
                }
            }
        }
    }

    uint find_dwarf_race_index() {
        uint dwarf_race_index = 0;
        int expected_dwarf_race_value = 166;
        foreach(uint ptr, scan_mem(QByteArray("A group of "))) {
            foreach(uint ptr2, scan_mem(encode(ptr))) {
                qDebug() << "\tPTR" << hex << ptr2 << read_str2(ptr2);
                QByteArray needle(2, 0);
                needle[0] = 0x66;
                needle[1] = 0x39;
                int offset = get_data(ptr2, 60).indexOf(needle);
                if (offset != -1) {
                    qDebug() << "\tMATCH! offset" << offset << hex << offset;
                    uint idx_addr = read_uint(ptr2 + offset + 3);
                    qDebug() << "\tREAD ADDR FROM" << hex << ptr2 + offset << "=" << idx_addr;
                    int idx = read_int32(idx_addr);
                    qDebug() << "\t\tRACE VALUE" << idx << "HEX" << hex << idx;
                    if (idx == expected_dwarf_race_value) {
                        dwarf_race_index = idx_addr;    
                        break;
                    }
                }    
            }
        }
        return dwarf_race_index;
    }

    QByteArray get_data(uint start_addr, int size) {
        char *buffer = new char[size];
        memset(buffer, 0, size);
        read_raw(start_addr, size, buffer);
        QByteArray data(buffer, size);
        delete[] buffer;
        return data;
    }

    //! ahhh convenience
    QString pprint(uint addr, int size) {
        return pprint(get_data(addr, size), addr);
    }

    QString pprint(const QByteArray &ba, const uint &start_addr = 0) {
    	QString out = "   ADDR  | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | TEXT\n";
    	out.append("------------------------------------------------------------------------\n");
    	int lines = ba.size() / 16;
    	if (ba.size() % 16)
    		lines++;
    
    	for(int i = 0; i < lines; ++i) {
    		uint offset = start_addr + i * 16;
    		out.append(QString("0x%1").arg(offset, 8, 16, QChar('0')));
    		out.append(" | ");
    		for (int c = 0; c < 16; ++c) {
    			out.append(ba.mid(i*16 + c, 1).toHex());
    			out.append(" ");
    		}
    		out.append("| ");
    		for (int c = 0; c < 16; ++c) {
    			QByteArray tmp = ba.mid(i*16 + c, 1);
    			if (tmp.at(0) == 0)
    				out.append(".");
    			else if (tmp.at(0) <= 126 && tmp.at(0) >= 32)
    				out.append(tmp);
    			else
    				out.append(tmp.toHex());
    		}
    		//out.append(ba.mid(i*16, 16).toPercentEncoding());
    		out.append("\n");
    	}
    	return out;
    }

    uint find_creature_vector() {
        /*
        uint creature_vector_addr = 0;

        QByteArray needle("FirstCreature");
        foreach(uint nickname, scan_mem(needle)) {
            qDebug() << "nickname found at" << hex << nickname;
            foreach(uint nickname_ptr, scan_mem(encode(nickname))) {
                qDebug() << "found ptr to nickname at" << hex << nickname_ptr;
                foreach(uint dwarf_list_start, scan_mem(encode(nickname_ptr - 4))) {
                    qDebug() << "possible creature vector start at" << hex << dwarf_list_start;
                    foreach(uint dwarf_list_vec_ptr, scan_mem(encode(dwarf_list_start))) {
                        qDebug() << "possible creature vector ptr at" << hex << dwarf_list_vec_ptr;
                        creature_vector_addr = dwarf_list_vec_ptr;
                        break;
                    }
                }
            }
        }
        */
        uint creature_vector_addr = 0x08F5A270;

        uint first_dwarf = 0;
        if (creature_vector_addr) {
            QVector<uint> creatures = enumerate_vector(creature_vector_addr);
            first_dwarf = creatures.at(0);
            qDebug() << "FIRST DWARF" << hex << first_dwarf;
        }

        if (first_dwarf) {
            // find custom prof offset
            qDebug() << "LOOKING FOR OFFSETS FOR DWARF" << read_str2(first_dwarf) << read_str2(first_dwarf + 4);
            QByteArray data = get_data(first_dwarf, 0x1000); // get the data around the dwarf pointer
            int prof_offset = -1;
            for(int offset = 0; offset < 0x1000; offset += 4) {
                uint ptr = read_uint(first_dwarf + offset);
                if (is_valid_address(ptr) && read_str2(first_dwarf + offset) == "FirstProfession") {
                    prof_offset = offset;
                    break;
                }
            }
            qDebug() << "\tCustom Profession offset" << hex << prof_offset;

            // find labors array
            QByteArray labor_pattern(102, 0);
            labor_pattern[0] = 0x1;
            labor_pattern[1] = 0x1;
            labor_pattern[2] = 0x1;
            labor_pattern[3] = 0x1;
            labor_pattern[4] = 0x1;
            labor_pattern[5] = 0x1;
            labor_pattern[6] = 0x1;
            labor_pattern[7] = 0x1;
            labor_pattern[8] = 0x1;
            labor_pattern[9] = 0x1;
            labor_pattern[17] = 0x1;
            labor_pattern[42] = 0x1;
            labor_pattern[68] = 0x1;
            labor_pattern[73] = 0x1;

            int labor_offset = data.indexOf(labor_pattern);
            qDebug() << "\tLabor array offset" << hex << labor_offset;

            // find skills vector
            QByteArray skillpattern_miner = encode_skillpattern(0, 3340, 4);
            QByteArray skillpattern_metalsmith = encode_skillpattern(29, 0, 2);
            QByteArray skillpattern_swordsman = encode_skillpattern(40, 462, 3);
            QByteArray skillpattern_pump_operator = encode_skillpattern(65, 462, 1);

            uint start = 0;
            uint end = 0;
            int target_total_skills = 8;
            for (int offset = 4; offset < 0x1000; offset += 4) {
                memcpy(&start, data.data() + offset, 4);
                memcpy(&end, data.data() + offset + 4, 4);
                if (is_valid_address(start) &&
                    is_valid_address(end) &&
                    end > start &&
                    (end - start) % 4 == 0 &&
                    (end - start) / 4 == target_total_skills) { 
                    // looks like an array
                    qDebug() << "\tPOSSIBLE SKILL VECTOR offset:" << hex << offset;
                    QVector<uint> addrs = enumerate_vector(first_dwarf + offset);
                    qDebug() << "real entries" << addrs.size();
                    if (addrs.size() == target_total_skills) {
                        foreach(uint entry, addrs) {
                            short type = read_short(entry);
                            short rating = read_short(entry + 0x4);
                            uint exp = read_uint(entry + 0x8);
                            qDebug() << "\t\tENTRY" << "TYPE" << type << "RATING" << rating << "EXP" << exp;
                        }
                    }
                }
            }
        }
        return creature_vector_addr;
    }

    QVector<uint> scan_mem(const QByteArray &needle) {
        QVector<uint> addresses;
        QByteArrayMatcher matcher(needle);

        foreach(MemorySegment *seg, m_regions) {
            int step = 0x1000;
            char *buffer = new char[step];
            if (!buffer) {
                qCritical() << "unable to allocate char buffer of" << seg->size << "bytes!";
                continue;
            }
            int steps = seg->size / step;
            if (seg->size % step)
                steps++;

            for(uint ptr = seg->start_addr; ptr < seg->end_addr; ptr += step) {
                if (ptr + step > seg->end_addr)
                    step = seg->end_addr - (ptr + step);
                
                memset(buffer, 0, step);
                int bytes_read = read_raw(ptr, step, buffer);
                if (bytes_read < step)
                    qWarning() << "tried to read" << step << "bytes starting at" << hex << ptr << "but only got" << dec << bytes_read;
            
                int idx = matcher.indexIn(QByteArray(buffer, bytes_read));
                if (idx != -1) {
                    addresses << uint(ptr + idx);
                }
            }
            delete[] buffer;
        }
        return addresses;
    }

    QString read_str2(const uint &addr) {
        uint buffer_addr = read_uint(addr);
        int upper_size = 1024;
        char *c = new char[upper_size];
        memset(c, 0, upper_size);
        read_raw(buffer_addr, upper_size, c);
        QString out = QString::fromLatin1(c);
        delete[] c;
        return out;
    }

    QString read_str(const uint &start_address) {
        uint buffer_addr = read_uint(start_address);
        uint len = read_uint(start_address + 0x0C);
        uint cap = read_uint(start_address + 0x10);
        //qDebug() << "attempting to read string at" << hex << start_address << "buffer starts at:" << buffer_addr << "LEN" << dec << len << "CAP" << cap;
        if (len < 1 || len > 1000 || cap < len) {
            // something is wrong
            return read_str2(start_address);
        }
        char *c = new char[len];
        read_raw(buffer_addr, len, c);
        QString out = QString::fromLatin1(c, len);
        delete[] c;
        return out;
    }

    uint read_uint(uint start_address) {
        uint retval = 0;
        read_raw(start_address, sizeof(uint), &retval);
        return retval;
    }

    int read_int32(uint start_address) {
        int retval = -1;
        read_raw(start_address, sizeof(int), &retval);
        return retval;
    }

    short read_short(uint address) {
        short retval = -1;
        read_raw(address, sizeof(short), &retval);
        return retval;
    }

    ushort read_ushort(uint address) {
        ushort retval = -1;
        read_raw(address, sizeof(ushort), &retval);
        return retval;
    }

    int read_raw(uint start_address, uint bytes, void *buffer) {
        if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) == -1) {
            // unable to attach
            perror("ptrace attach");
            qCritical() << "Could not attach to PID" << m_pid;
            return 0;
        }
        int status;
        wait(&status);

        // read this procs base address for the ELF header
        QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
        if (!mem_file.open(QIODevice::ReadOnly)) {
            qCritical() << "Unable to open" << mem_file.fileName();
            ptrace(PTRACE_DETACH, m_pid, 0, 0);
            return 0;
        }

        uint bytes_read = 0;
        QByteArray data;
        while (bytes_read < bytes) {
            //qDebug() << "reading raw from:" << hex << start_address + bytes_read;
            mem_file.seek(start_address + bytes_read);
            QByteArray tmp = mem_file.read(bytes - data.size());
            bytes_read += tmp.size();
            data.append(tmp);
            if (bytes_read == 0) {
                qWarning() << "read 0 bytes from" << hex << start_address + bytes_read;
                break;
            }
            //qDebug() << "bytes_read:" << bytes_read;
        }
        memcpy(buffer, data.data(), data.size());
        mem_file.close();
        ptrace(PTRACE_DETACH, m_pid, 0, 0);
        return bytes_read;
    }

    void scan() {
        // read the maps file to find the range
        m_regions.clear();
        qDebug() << "scanning m_pid" << m_pid;
        QFile f(QString("/proc/%1/maps").arg(m_pid));
        if (!f.open(QIODevice::ReadOnly)) {
            qCritical() << "Unable to open" << f.fileName();
            return;
        }
        qDebug() << "opened" << f.fileName();
        QByteArray line;
        uint lowest_addr = 0xFFFFFFFF;
        uint start_addr = 0;
        uint end_addr = 0;
        bool ok;

        QRegExp rx("^([a-f\\d]+)-([a-f\\d]+)\\s([rwxsp-]{4})\\s+[\\d\\w]{8}\\s+[\\d\\w]{2}:[\\d\\w]{2}\\s+(\\d+)\\s*(.+)\\s*$");
        do {
            line = f.readLine();
            //qDebug() << "parsing:" << QString(line).trimmed();
            // parse the first line to see find the base
            if (rx.indexIn(line) != -1) {
                //qDebug() << "RANGE" << rx.cap(1) << "-" << rx.cap(2) << "PERMS" << rx.cap(3) << "INODE" << rx.cap(4) << "PATH" << rx.cap(5);
                start_addr = rx.cap(1).toUInt(&ok, 16);
                end_addr = rx.cap(2).toUInt(&ok, 16); 
                QString perms = rx.cap(3).trimmed();
                int inode = rx.cap(4).toInt();
                QString path = rx.cap(5).trimmed();

                //qDebug() << "RANGE" << hex << start_addr << "-" << end_addr << perms << inode << "PATH >" << path << "<";
                bool keep_it = false;
                if (path.contains("[heap]") || path.contains("[stack]") || path.contains("[vdso]"))  {
                    keep_it = true;
                } else if (perms.contains("r") && inode && path == QFile::symLinkTarget(QString("/proc/%1/exe").arg(m_pid))) {
                    keep_it = true;
                } else {
                    keep_it = path.isEmpty();
                }
                // uncomment to search HEAP only
                //keep_it = path.contains("[heap]");
                keep_it = true;

                if (keep_it) {
                    //qDebug() << "KEEPING RANGE" << hex << start_addr << "-" << end_addr << "PATH " << path;
                    m_regions << new MemorySegment(path, start_addr, end_addr);
                    if (start_addr < lowest_addr)
                        lowest_addr = start_addr;
                }

                if (path.contains("[heap]"))
                    m_heap_start = start_addr;
            }
        } while (!line.isEmpty());
        f.close();
        //qDebug() << "LOWEST ADDR:" << hex << lowest_addr;

        /*
        //DUMP LIST OF MEMORY RANGES
         QPair<uint, uint> tmp_pair;
        foreach(tmp_pair, m_regions) {
            qDebug() << "RANGE start:" << hex << tmp_pair.first << "end:" << tmp_pair.second;
        }
        */

        // attach to m_pid
        if (ptrace(PTRACE_ATTACH, m_pid, 0, 0) == -1) {
            qCritical() << "unable to attach to process" << m_pid;
            return;
        }
        int status;
        wait(&status);
        qDebug() << "attached to process" << m_pid;

        // read this procs base address for the ELF header
        QFile mem_file(QString("/proc/%1/mem").arg(m_pid));
        if (!mem_file.open(QIODevice::ReadOnly)) {
            qCritical() << "Unable to open" << mem_file.fileName();
            return;
        }
        mem_file.seek(lowest_addr + 0x18); // this should be the entry point in the ELF header
        QByteArray data = mem_file.read(4);
        mem_file.close();
        memcpy(&m_base_addr, data.data(), sizeof(int));

    
        qDebug() << "base_addr:" << hex << m_base_addr;
        m_memory_correction = (int)m_base_addr - 0x400000;
        qDebug() << "memory correction:" << hex << m_memory_correction;

        ptrace(PTRACE_DETACH, m_pid, 0, 0);
    }
};

int main(int argc, char** argv) {
    MemoryReader mr(argc, argv);
}
