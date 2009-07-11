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
