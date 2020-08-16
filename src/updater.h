/*
Dwarf Therapist
Copyright (c) 2020 Clement Vuchener

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
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "notifierwidget.h"
#include "version.h"

class Updater: public QObject {
    Q_OBJECT
public:
    Updater(QObject *parent = nullptr);
    ~Updater() override;

public slots:
    void check_for_updates();

signals:
    void notify(NotifierWidget::notify_info);
    void latest_version(const Version &version, const QString &url);
    void layout_update_data(const QString &name, const QString &git_sha, const QByteArray &data);

private:
    void network_error(QNetworkReply::NetworkError);
    void ssl_errors(const QList<QSslError> &);
    void latest_release_received();
    void layout_list_received();

    QNetworkAccessManager m_network;
};

#endif
