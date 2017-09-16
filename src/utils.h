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

#include <QColor>
#include <QVariant>
#include <QPixmap>
#include <QBuffer>
#include <QIODevice>
#include <QString>
#include <math.h>

// meh
typedef quint64 VIRTADDR;
typedef quint64 USIZE;
typedef qint64 SSIZE;
typedef quint8 BYTE;
typedef quint16 WORD;

static inline QColor complement(const QColor &in_color, float brightness_threshold = 0.50) {
    qreal brightness = sqrt(pow(in_color.redF(),2.0) * 0.241 +
                            pow(in_color.greenF(),2.0) * 0.691 +
                            pow(in_color.blueF(),2.0) * 0.068);
    return QColor::fromHsv(in_color.toHsv().hue(), 25, brightness >= brightness_threshold || in_color.alpha() < 130 ? 0 : 255);
}

static inline QString hexify(const quint64 &num) {
    int width = 8;
    if (num >> 32)
        width = 16;
    return QString("0x%1").arg(num, width, 16, QChar('0'));
}
static inline QString hexify(const QByteArray &bytes) {
    return QString("0x%1").arg(QString(bytes.toHex()), 8, QChar('0'));
}

static inline QString capitalize(const QString & word) {
    QString result = word;
    if(!result.isEmpty()) {
        bool in_tag = false;
        int idx = 0;
        for(idx=0;idx < word.size();idx++){
            if(!in_tag && word.at(idx) == '<')
                in_tag = true;
            else if(in_tag && word.at(idx) == '>')
                in_tag = false;
            if(!in_tag && word.at(idx).isLetter())
                break;
        }
        if(idx == word.size()-1)
            idx = 0;
        if(result[idx].isLetter())
            result[idx] = result[idx].toUpper();
    }
    return result;
}

static inline QString capitalizeEach(const QString & word){
    QStringList list = word.split(" ");
    for(int i = 0; i < list.length(); i++){
        list[i] = capitalize(list[i]);
    }
    return list.join(" ");
}

template <class T> class vPtr
{
public:
    static inline T* asPtr(QVariant v){
        return static_cast<T*>(v.value<void *>());
    }
    static inline QVariant asQVariant(T* ptr){
        return qVariantFromValue(static_cast<void*>(ptr));
    }
};

static inline QString embedPixmap(const QPixmap &img){
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    return QString("<img src=\"data:image/png;base64,%1\"/>").arg(QString(buffer.data().toBase64()));
}

static inline QString formatList(QStringList values){
    QString ret_val = "";
    QString last_msg;
    if(values.count() > 1)
        last_msg = values.takeLast();
    ret_val = values.join(", ");
    if(!last_msg.isEmpty()){
        ret_val.append(QObject::tr(" and ") + last_msg);
    }
    return ret_val;
}

static inline QString formatNumber(double value, bool useSI) {
    if(useSI){
        QString suffixes(QObject::tr("kMGT"));
        for(int idx = suffixes.length(); idx > 0; idx--){
            double unit = pow(1000,idx);
            if(fabs(value) >= unit)
                return QString("%L1%2").arg(value/unit,0,'f',1).arg(suffixes.at(idx-1));
        }
    }
    return QString("%L1").arg(value);
}

static inline QColor read_color(QString const &col){
    QColor c(Qt::gray);
    bool ok;
    if(col.startsWith("0x")){
        int a = 255;
        if(col.length() >= 8){
            if(col.length() >= 10)
                a = col.mid(8, 2).toInt(&ok, 16);
            c = QColor(col.mid(2,6).toInt(&ok, 16));
            c.setAlpha(a);
        }
    }else{
        c = QVariant(col).value<QColor>();
    }
    return c;
}

#endif // UTILS_H
