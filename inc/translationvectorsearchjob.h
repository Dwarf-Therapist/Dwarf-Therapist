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
#ifndef TRANSLATION_VECTOR_SEARCH_JOB_H
#define TRANSLATION_VECTOR_SEARCH_JOB_H

#include "scannerjob.h"
#include "defines.h"
#include "gamedatareader.h"
#include "utils.h"

class TranslationVectorSearchJob : public ScannerJob {
	Q_OBJECT
public:
	TranslationVectorSearchJob() 
		: ScannerJob(FIND_TRANSLATIONS_VECTOR)
	{}
	public slots:
		void go() {
			if (!m_ok) {
				LOGC << "Scanner Thread couldn't connect to DF!";
				emit quit();
				return;
			}
			LOGD << "Starting Search in Thread" << QThread::currentThreadId();

			emit main_scan_total_steps(4);
			emit main_scan_progress(0);

			GameDataReader *gdr = GameDataReader::ptr();
			//! number of words that should be in a single language
			int target_total_words = gdr->get_int_for_key("ram_guesser/total_words_per_table", 10);
			//! total number of languages we should find
			int target_total_langs = gdr->get_int_for_key("ram_guesser/total_languages", 10);
			//! first word in the non-translated language
			QString first_generic_word = gdr->get_string_for_key("ram_guesser/first_generic_word");
			//! first word in the dwarf translation
			QString first_dwarf_word = gdr->get_string_for_key("ram_guesser/first_dwarf_word");

			// we'll fill these in as we find them
			uint dwarf_translation = 0; // address of the DWARF translation object
			uint dwarf_lang_table = 0; // address of the vector of words inside the above object
			uint word_table_offset = 0; // offset from dwarf_translation to the dwarf_lang_table
			QVector<uint> translations_vectors; // as we find candidate pointers we'll put them in here

			emit scan_message(tr("Scanning for vectors with %1 entries").arg(target_total_words));
			emit main_scan_progress(1);
			foreach(uint addr, m_df->find_vectors(target_total_words)) {
				foreach(uint vec_addr, m_df->enumerate_vector(addr)) {
					QString first_entry = m_df->read_string(vec_addr);
					if (first_entry == first_generic_word) {
						emit found_address("generic_lang_table", addr);
					} else if (first_entry == first_dwarf_word) {
						LOGD << "FOUND DWARF LANG TABLE" << hex << addr;
						dwarf_lang_table = addr; // store this
					} else {
						break;
					}
					break;
				}
			}
			//dwarf_lang_table = 0x00314abc;

			emit scan_message(tr("Scanning for DWARF buffer"));
			emit main_scan_progress(2);

			if (dwarf_lang_table) {
				int steps = 0x100 / 4;
				emit sub_scan_total_steps(steps);
				emit sub_scan_progress(1);
				for (int i = 0; i < steps; ++i) {
					uint addr = dwarf_lang_table - (i * 4);
					if (m_df->read_string(addr) == "DWARF") {
						dwarf_translation = addr - m_df->VECTOR_POINTER_OFFSET;
						word_table_offset = dwarf_lang_table - dwarf_translation;
						emit found_offset("word_table", word_table_offset);
						//now find a pointer to this guy...
						foreach (uint trans_ptr, m_df->scan_mem(encode(dwarf_translation + m_df->VECTOR_POINTER_OFFSET))) {
							foreach (uint trans_vec_ptr, m_df->scan_mem(encode(trans_ptr))) {
								translations_vectors << trans_vec_ptr - m_df->VECTOR_POINTER_OFFSET;
							}
						}
						emit sub_scan_progress(steps);
						break;
					}
					emit sub_scan_progress(i);
				}
			}
			emit scan_message(tr("Verifying found vectors"));
			emit main_scan_progress(3);
			emit sub_scan_total_steps(translations_vectors.size());
			emit sub_scan_progress(1);
			int count = 0;
			foreach(uint vec, translations_vectors) {
				if (m_df->looks_like_vector_of_pointers(vec)) {
					QVector<uint> langs = m_df->enumerate_vector(vec);
					LOGD << "ENTRIES" << langs.size();
					if (langs.size() == target_total_langs) {
						if (m_df->read_string(langs.at(0)) == "DWARF") {
							LOGD << "+++ VERIFIED +++";
							LOGD << "\tTRANSLATIONS VECTOR AT" << hex << vec;
							LOGD << "\tDWARF TRANS OBJECT" << hex << langs.at(0) << "WORD TABLE" << dwarf_lang_table;
							LOGD << "\tOFFSET FROM WORD TABLE" << hex << word_table_offset;
							LOGD << "\tFINAL ADDRESS" << QString("0x%1").arg(vec - m_df->get_memory_correction(), 8, 16, QChar('0'));
							emit found_address("translation_vector", vec);
							break;
						}
					}
				}
				emit sub_scan_progress(++count);
			}
			emit main_scan_progress(4);
			emit quit();
		}
};
#endif