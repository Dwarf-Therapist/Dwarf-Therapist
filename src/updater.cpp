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

#include "updater.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRegularExpression>

#include "defines.h"
#include "dwarftherapist.h"
#include "standardpaths.h"
#include "truncatingfilelogger.h"

#include "qt_compat.h"

Updater::Updater(QObject *parent)
    : QObject(parent)
{
    connect(&m_network, &QNetworkAccessManager::finished,
            [](QNetworkReply *reply) { reply->deleteLater(); });
}

Updater::~Updater()
{
}

void Updater::check_for_updates()
{
    if (!m_network.networkAccessible()) {
        notify({tr("Network Inaccessible"),
                tr("Could not check for updates due to network inaccessibility"),
                {}, {}, true});
        return;
    }
    auto settings = StandardPaths::settings();
    auto reply = m_network.get(QNetworkRequest(QString("https://api.github.com/repos/%1/%2/releases/latest")
                    .arg(settings->value("update_repo_owner", REPO_OWNER).toString())
                    .arg(settings->value("update_repo_name", REPO_NAME).toString())));
    LOGD << "Query latest release version from" << reply->request().url();
    connect(reply, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error),
            this, &Updater::network_error);
    connect(reply, &QNetworkReply::sslErrors,
            this, &Updater::ssl_errors);
    connect(reply, &QNetworkReply::finished,
            this, &Updater::latest_release_received);
}

void Updater::network_error(QNetworkReply::NetworkError)
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;
    LOGE << "Network error:" << reply->errorString();
    notify({tr("Network Error"),
            reply->errorString(),
            DT->project_homepage(),
            tr("Click here to go the project home page."),
            true});
}

void Updater::ssl_errors(const QList<QSslError> &errors)
{
    QStringList error_strings;
    for (auto error: errors)
        error_strings.push_back(error.errorString());
    LOGE << "SSL errors:" << error_strings.join(", ");
    notify({tr("SSL Errors"),
            error_strings.join(", "),
            {}, {}, true});
}

void Updater::latest_release_received()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    LOGD << "Latest version manifest received";
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        LOGE << "Failed to parse JSON for release manifest:" << error.errorString();
        notify({tr("Failed to check latest version"),
                tr("Invalid json data: %1").arg(error.errorString()),
                {}, {}, true});
        return;
    }
    auto version_string = doc.object().value("tag_name").toString();
    auto url = doc.object().value("html_url").toString();
    QRegularExpression version_re("(\\d+)\\.(\\d+)\\.(\\d+)");
    auto match = version_re.match(version_string);
    if (!match.hasMatch()) {
        LOGE << "Invalid version format:" << version_string;
        notify({tr("Failed to parse latest version"),
                tr("\"%1\" is not in the expected format").arg(version_string),
                url,
                tr("Click here to view the latest release."),
                true});
        return;
    }
    Version latest{
        match.captured(1).toInt(),
        match.captured(2).toInt(),
        match.captured(3).toInt()
    };
    LOGD << "Latest version is: " << latest.to_string();
    latest_version(latest, url);

    // Now that the latest version was successfully checked,
    // chain with memory layouts checks
    auto settings = StandardPaths::settings();
    reply = m_network.get(QNetworkRequest(QString("https://api.github.com/repos/%1/%2/contents/share/memory_layouts/%3")
                    .arg(settings->value("update_repo_owner", REPO_OWNER).toString())
                    .arg(settings->value("update_repo_name", REPO_NAME).toString())
                    .arg(LAYOUT_SUBDIR)));
    LOGD << "Querying memory layouts from repository" << reply->request().url();
    connect(reply, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error),
            this, &Updater::network_error);
    connect(reply, &QNetworkReply::sslErrors,
            this, &Updater::ssl_errors);
    connect(reply, &QNetworkReply::finished,
            this, &Updater::layout_list_received);
}

void Updater::layout_list_received()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    QJsonParseError error;
    auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        LOGE << "Failed to parse JSON for memory layout list:" << error.errorString();
        notify({tr("Failed to check memory layouts"),
                tr("Invalid json data: %1").arg(error.errorString()),
                {}, {}, true});
        return;
    }
    for (auto item: doc.array()) {
        auto file = item.toObject();
        QUrl url = file.value("download_url").toString();
        QString git_sha = file.value("sha").toString();
        auto reply = m_network.get(QNetworkRequest(url));
        LOGD << "Downloading memory layout from repository" << reply->request().url();
        connect(reply, qOverload<QNetworkReply::NetworkError>(&QNetworkReply::error),
                this, &Updater::network_error);
        connect(reply, &QNetworkReply::sslErrors,
                this, &Updater::ssl_errors);
        connect(reply, &QNetworkReply::finished,
                [=](){ layout_update_data(url.fileName(), git_sha, reply->readAll()); });
    }
}

