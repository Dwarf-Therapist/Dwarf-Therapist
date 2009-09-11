#ifndef MEMORY_SEGMENT_H
#define MEMORY_SEGMENT_H

struct MemorySegment {
	MemorySegment(const QString &_name, const uint &_start_addr, const uint &_end_addr) 
		: name(_name)
		, start_addr(_start_addr)
		, end_addr(_end_addr)
		, is_heap(false)
		, is_guarded(false)
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

	//! check to see if an address is contained in this memory segment
	bool contains(const uint &addr) {
		return addr >= start_addr && addr <= end_addr;
	}

	uint size;
	QString name;
	uint start_addr;
	uint end_addr;
	bool is_heap;
	bool is_guarded; // only used on windows right now
};

#endif