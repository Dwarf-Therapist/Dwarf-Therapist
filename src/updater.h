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

#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QNetworkReply>
#include "notifierwidget.h"

class DFInstance;
struct Version;

class Updater : public QObject {
    Q_OBJECT
public:
    Updater(QObject *parent = 0);
    ~Updater();

    QString last_updated_checksum(){return m_last_updated_checksum;}

public slots:
    bool network_accessible(const QString &name);
    void check_latest_version(bool notify_on_ok = true);
    void version_check_finished();
    void check_layouts(DFInstance *df);
    void layout_downloaded();
    void update_error(QNetworkReply::NetworkError);

signals:
    void notify(NotifierWidget::notify_info);

protected:
    DFInstance *m_df;
    QNetworkAccessManager *m_network;
    QHash<QString,int> m_layout_queue;
    QString m_last_updated_checksum;

    bool load_manifest(QNetworkReply *reply, QVariant &manifest_object);
    QStringList find_layout_urls(QNetworkReply *reply);
    void load_latest_version(QNetworkReply *reply, Version &latest_version, QString &url);
    void load_layout_data(QString data, QStringList &list, QRegExp rx);
    void show_notification(QNetworkReply *reply);

};
#endif // UPDATER_H
