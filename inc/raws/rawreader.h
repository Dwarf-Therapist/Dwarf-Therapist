/*
Dwarf Therapist
Copyright (c) 2011 Justin Ehlert (Dwarf Engineer)

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
#ifndef RAW_READER_H
#define RAW_READER_H

#include <QFileInfo>
#include <QStringList>
#include "raws/rawobject.h"
#include "raws/rawobjectlist.h"

class RawReader {
private:
    RawReader() {}

public:
    static QRawObjectList read_objects(QFileInfo file);

private:
    static RawObjectPtr read_object(QStringList & lines);
    static void populate_node_values(const RawNodePtr & node, QString & line);
    static void populate_sub_nodes(const RawNodePtr & node, QStringList & lines, QString indent = "\t");
};

#endif // RAW_READER_H
