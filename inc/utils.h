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
#ifndef UTILS_H
#define UTILS_H

#include <QByteArray>

static inline QByteArray encode_short(short num) {
    char bytes[2];
    bytes[0] = (char)num;
    bytes[1] = (char)(num >> 8);
    QByteArray arr(bytes, 2);
    return arr;
}

static inline QByteArray encode_int(int num) {
    char bytes[4];
    bytes[0] = (char)num;
    bytes[1] = (char)(num >> 8);
    bytes[2] = (char)(num >> 16);
    bytes[3] = (char)(num >> 24);
    QByteArray arr(bytes, 4);
    return arr;
}

static inline QByteArray encode_skillpattern(short skill, short exp, short rating) {
    QByteArray bytes;
    bytes.reserve(6);
    bytes[0] = (uchar)skill;
    bytes[1] = (uchar)(skill >> 8);
    bytes[2] = (uchar)exp;
    bytes[3] = (uchar)(exp >> 8);
    bytes[4] = (uchar)rating;
    bytes[5] = (uchar)(rating >> 8);
    return bytes;
}

static inline QString by_char(QByteArray arr) {
    QString out;
    for(int i=0; i < arr.size(); i++) {
        out += QString::number((uchar)arr.at(i), 16);
        out += " ";
    }
    return out;
}

#endif // UTILS_H
