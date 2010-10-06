/*
Dwarf Therapist
Copyright (c) 2010 Justin Ehlert (Hecaterus)

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
#include "layoutcreator.h"
#include "dfinstance.h"
#include "scannerthread.h"
#include "memorylayout.h"

LayoutCreator::LayoutCreator(DFInstance * df, MemoryLayout * parent, QString file_name, QString version_name) :
        m_df(df),
        m_parent(parent),
        m_file_name(file_name),
        m_version_name(version_name),
        m_dwarf_race_index(0),
        m_translation_vector(0),
        m_language_vector(0),
        m_creature_vector(0)
{

}

LayoutCreator::~LayoutCreator()
{

}

bool LayoutCreator::write_file()
{
    // Copy the file to the new location
    QFile file(m_parent->filename());
    if(!file.copy(m_file_name)) {
        //TODO: Error handling
        LOGD << "\tError copying file to new location!";
        return false;
    }

    // Read the file

    // Update values
    MemoryLayout newLayout(m_file_name, m_parent->data());
    QString checksum = hexify(m_df->calculate_checksum());
    newLayout.set_game_version(m_version_name);
    newLayout.set_checksum(checksum);
    newLayout.set_address("addresses/translation_vector", m_translation_vector);
    newLayout.set_address("addresses/language_vector", m_language_vector);
    newLayout.set_address("addresses/creature_vector", m_creature_vector);
    newLayout.set_address("addresses/dwarf_race_index", m_dwarf_race_index);
    newLayout.set_complete();
    LOGD << "\tWriting file.";
    newLayout.save_data();
    return true;
}


void LayoutCreator::report_address(const QString& name, const quint32& addr)
{
    VIRTADDR corrected_addr = addr - m_df->get_memory_correction();

    if(name == "Dwarf Race" && m_dwarf_race_index == 0)
    {
        m_dwarf_race_index = corrected_addr;
    }
    else if(name == "language_vector" && m_language_vector == 0)
    {
        m_language_vector = corrected_addr;
    }
    else if(name == "translation_vector" && m_translation_vector == 0)
    {
        m_translation_vector = corrected_addr;
    }
    else if(name == "creature_vector" && m_creature_vector == 0)
    {
        m_creature_vector = corrected_addr;
    }
}
