#include "updater.h"
#include "notifierwidget.h"
#include "dwarftherapist.h"
#include "defines.h"
#include "utils.h"
#include "truncatingfilelogger.h"
#include "mainwindow.h"
#include "memorylayout.h"
#include "dfinstance.h"
#include "standardpaths.h"
#include "version.h"

#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Updater::Updater(QObject *parent)
    : QObject(parent)
    , m_df(0)
    , m_last_updated_checksum("")
{
}

Updater::~Updater(){
    m_df = 0;
}

bool Updater::network_accessible(const QString &name){
    if(m_network.networkAccessible() != QNetworkAccessManager::NotAccessible){
        return true;
    }else{
        NotifierWidget::notify_info ni;
        ni.title = tr("Network Inaccessible");
        ni.is_warning = true;
        ni.details = tr("Could not check for %1 due to network inaccessibility").arg(name);
        notify(ni);
        return false;
    }
}

void Updater::check_latest_version(bool notify_on_ok) {
    if(network_accessible(tr("new releases"))){
        QNetworkReply *reply = m_network.get(QNetworkRequest(QUrl(QString("https://api.github.com/repos/%1/%2/releases/latest")
                                                                  .arg(DT->user_settings()->value("update_repo_owner",REPO_OWNER).toString())
                                                                  .arg(DT->user_settings()->value("update_repo_name",REPO_NAME).toString()))));
        reply->setProperty("release_check",true);
        if(notify_on_ok){
            reply->setProperty("notify_on_ok",true);
        }
        connect(reply,SIGNAL(finished()),this,SLOT(version_check_finished()));
        connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(update_error(QNetworkReply::NetworkError)));
    }
}

void Updater::update_error(QNetworkReply::NetworkError err){
    if(err == QNetworkReply::NoError)
        return;
    show_notification(qobject_cast<QNetworkReply*>(sender()));
}

void Updater::show_notification(QNetworkReply *reply){
    NotifierWidget::notify_info ni;
    bool release_check = false;
    QString err_msg = reply->errorString();
    //use our custom error message if it's been set
    if(reply->property("ERROR").isValid()){
        err_msg = reply->property("ERROR").toString();
    }

    if(reply->property("release_check").isValid()){
        release_check = reply->property("release_check").toBool();
    }

    if(release_check){
        ni.title = tr("Version Check Error");
        ni.url = DT->project_homepage();
        ni.url_msg = tr("Click here to go to the latest release page.");
    }else{
        ni.title = tr("Memory Layout Error");
        ni.url = QString("%1/releases/latest").arg(DT->project_homepage());
        ni.url_msg = tr("Click here to go to the project home page. Navigate to <i>share/memory_layouts</i> to manually update.");
        m_layout_queue.remove(reply->url().toString());
    }
    ni.is_warning = true;
    ni.details = capitalize(err_msg);
    notify(ni);

    reply->deleteLater();
}

void Updater::version_check_finished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());

    NotifierWidget::notify_info ni;
    ni.title = "";
    ni.url = QString("%1/releases/latest").arg(DT->project_homepage());
    ni.url_msg = tr("Click here to go to the latest release page.");
    ni.is_warning = false;

    bool notify_on_ok = false;
    notify_on_ok = reply->property("notify_on_ok").isValid();

    if(reply->error() == QNetworkReply::NoError){
        Version v_current;
        Version v_latest(0,0,0);
        QString url;
        load_latest_version(reply,v_latest,url);
        if(v_current < v_latest){
            LOGI << "New version found" << v_latest.to_string();
            ni.title = tr("New Version Available");
            ni.url_msg = tr("Click here to download version %1").arg(v_latest.to_string());
            ni.url = url;
        }else if(notify_on_ok){
            ni.title = tr("Latest Version");
            ni.details = tr("This version is the latest release.");
            ni.url = "";
            ni.url_msg = "";
        }
    }

    if(!ni.title.trimmed().isEmpty()){
        notify(ni);
    }

    reply->deleteLater();
}

void Updater::check_layouts(DFInstance *df) {
    m_df = df;
    LOGI << "Checking for layout for checksum: " << m_df->df_checksum();
    if(!network_accessible(tr("memory layouts"))){
        m_df = 0;
        return;
    }

    //load a list of all layout files from the repo
    QNetworkReply *reply = m_network.get(QNetworkRequest(QUrl(QString("https://api.github.com/repos/%1/%2/contents/share/memory_layouts/%3")
                                                              .arg(DT->user_settings()->value("update_repo_owner",REPO_OWNER).toString())
                                                              .arg(DT->user_settings()->value("update_repo_name",REPO_NAME).toString())
                                                              .arg(m_df->layout_subdir()))));
    QEventLoop manifest_loop;
    connect(reply, SIGNAL(finished()),&manifest_loop,SLOT(quit()));
    connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(update_error(QNetworkReply::NetworkError)));
    manifest_loop.exec();

    if(reply->error() == QNetworkReply::NoError){
        QStringList layout_urls = find_layout_urls(reply);
        if(layout_urls.count() > 0){
            DT->get_main_window()->set_progress_message(tr("Downloading memory layouts..."));
            DT->get_main_window()->set_progress_range(0,layout_urls.size());
            DT->get_main_window()->set_progress_value(0);

            QEventLoop layout_dl;
            foreach(QString layout_url, layout_urls){
                QNetworkReply *dl_reply = m_network.get(QNetworkRequest(QUrl(layout_url)));
                m_layout_queue.insert(dl_reply->url().toString(),0);
                connect(dl_reply,SIGNAL(finished()),this,SLOT(layout_downloaded()));
                connect(dl_reply,SIGNAL(finished()),&layout_dl,SLOT(quit()));
                connect(dl_reply,SIGNAL(error(QNetworkReply::NetworkError)),this,SLOT(update_error(QNetworkReply::NetworkError)));
            }
            layout_dl.exec();

            while(!m_layout_queue.isEmpty()){
                QCoreApplication::processEvents(QEventLoop::AllEvents,100);
                DT->get_main_window()->set_progress_value(layout_urls.size()-m_layout_queue.size());
            }
        }
    }
    reply->deleteLater();
    m_df = 0;
}

bool Updater::load_manifest(QNetworkReply *reply, QVariant &manifest_object){

    QJsonParseError *json_err = new QJsonParseError();
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(),json_err);
    if(json_err && json_err->error != QJsonParseError::NoError){
        LOGW << json_err->errorString();
        //emit a reply error to be handled by update_error
        reply->setProperty("ERROR",json_err->errorString());
        reply->error(QNetworkReply::UnknownContentError);
        return false;
    }else{
        manifest_object = doc;
    }
    return true;
}

QStringList Updater::find_layout_urls(QNetworkReply *reply){
    QStringList layout_urls;
    QVariant manifest_object;
    if(load_manifest(reply,manifest_object)){
        QJsonDocument manifest = qvariant_cast<QJsonDocument>(manifest_object);
        QJsonArray file_infos =  manifest.array();
        for(int idx = 0; idx < file_infos.size(); idx++){
            QJsonObject file_info = file_infos.at(idx).toObject();
            if(!file_info.isEmpty()){
                //download layouts that either we don't have, or have a different SHA
                //we'll still have to read them and compare their checksum with the current df checksum
                //but this will reduce the number of layouts we need to check
                QString git_sha = file_info.value("sha").toString();
                if(!m_df->find_memory_layout(git_sha)){
                    layout_urls.prepend(file_info.value("download_url").toString());
                    LOGD << "downloading layout" << file_info.value("name").toString() << "SHA:" << git_sha;
                }
            }
        }
    }
    return layout_urls;
}

void Updater::load_latest_version(QNetworkReply *reply, Version &latest_version, QString &url){
    QRegExp rx_version;
    QString data;
    QVariant manifest_object;
    if(load_manifest(reply,manifest_object)){
        QJsonDocument manifest = qvariant_cast<QJsonDocument>(manifest_object);
        QJsonObject release_info = manifest.object();
        if(!release_info.isEmpty()){
            data = release_info.value("tag_name").toString();
            rx_version.setPattern("(\\d+)\\.(\\d+)\\.(\\d+)");
            url = release_info.value("html_url").toString();
        }
        int idx = rx_version.indexIn(data);
        if(idx != -1){
            latest_version.major = rx_version.cap(1).toInt();
            latest_version.minor = rx_version.cap(2).toInt();
            latest_version.patch = rx_version.cap(3).toInt();
        }
    }

}

void Updater::load_layout_data(QString data, QStringList &list, QRegExp rx){
    int pos = 0;
    while ((pos = rx.indexIn(data, pos)) != -1) {
        list << rx.cap(1);
        pos += rx.matchedLength();
    }
}

void Updater::layout_downloaded() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
    NotifierWidget::notify_info ni;
    ni.title = tr("Memory Layout Failed");
    ni.url_msg = "";
    ni.url = "";
    ni.is_warning = true;

    if(reply->error() == QNetworkReply::NoError){
        QString layout_data = QString(reply->readAll());
        QRegExp rx("checksum\\s*=\\s*(0[xX][0-9a-fA-F]+)");
        QString checksum = "";
        if(rx.indexIn(layout_data) > -1){
            checksum = rx.cap(1);
        }
        //only update/add layouts for the current version
        if(checksum == m_df->df_checksum()){
            //see if we have an existing layout to update
            MemoryLayout *m = m_df->get_memory_layout(checksum);
            if(m && m->filepath().startsWith(StandardPaths::writable_data_location())){
                //if we already have a writable layout with this checksum, update the layout
                QFile file(m->filepath());
                if(file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                    QTextStream layout(&file);
                    layout << layout_data;
                    file.close();
                    m->load_data();
                    ni.title = tr("Memory Layout Updated");
                    ni.details = tr("Memory layout %1 has been updated!").arg(m->filename());
                    ni.is_warning = false;
                }else{
                    ni.title = tr("Access Denied");
                    ni.details = tr("Could not open %1 to update the layout!").arg(m->filepath());
                }
            }else{
                QString dl_filename = reply->url().fileName();
                QString add_result;
                if(m_df->add_new_layout(dl_filename,layout_data,add_result)){
                    ni.title = tr("Memory Layout Added");
                    ni.details = tr("A new memory layout (%1) has been downloaded!").arg(dl_filename);
                    ni.is_warning = false;
                }else{
                    ni.details = tr("Failed to create a new memory layout for %1!<br><br>%2").arg(dl_filename).arg(add_result);
                }
            }
            m_last_updated_checksum = checksum;
        }
    }

    if(!ni.details.trimmed().isEmpty()){
        notify(ni);
    }

    reply->deleteLater();
    m_layout_queue.remove(reply->url().toString());
}
