/*
 * Copyright (c) 2013-2014 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cardui.hpp"

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/system/Clipboard.hpp>
#include <bb/system/CardDoneMessage>
#include "database.hpp"
#include "Uploader.hpp"
using namespace bb::cascades;
using namespace bb::system;
const QString CardUI::m_author = "Anwesh"; // for creating settings
const QString CardUI::m_appName = "Slack"; // for creating settings
CardUI::CardUI(bb::system::InvokeManager* invokeManager) :
        ApplicationUIBase(invokeManager), settingsWatcher(new QFileSystemWatcher(this)), db(
                new Database(this)), dm(new DownloadManager(this)) ,uploader(new Uploader(this))
{
    // Create scene document from main.qml asset, the parent is set
    // to ensure the document gets destroyed properly at shut down.

    QSettings settings(m_author, m_appName);
    // Force the creation of the settings file so that we can watch it for changes.
    settings.sync();
    // Watcher for changes in the settings file.
    settingsWatcher = new QFileSystemWatcher(this);
    settingsWatcher->addPath(settings.fileName());

    connect(settingsWatcher, SIGNAL(fileChanged(const QString&)), this,
            SLOT(settingsChanged(const QString&)));
    connect(m_pInvokeManager, SIGNAL(invoked(const bb::system::InvokeRequest&)), this,
            SLOT(onInvoked(const bb::system::InvokeRequest&)));

    // Connect to the "cardPooled" signal to received notifications when the card is placed in the pool
    connect(m_pInvokeManager, SIGNAL(cardPooled(const bb::system::CardDoneMessage&)), this,
            SLOT(cardPooled(const bb::system::CardDoneMessage&)));
    QObject::connect(dm, SIGNAL(requestCall()), this, SLOT(onRequestCall()));


}

void CardUI::cardPooled(const bb::system::CardDoneMessage& doneMessage)
{
    // Card is no longer being shown and has been pooled
    // The card process is still running, but has been pooled so that future invocations are optimized.
    // Therefore, when the card receives this signal, it must reset its state so that it is ready
    // to be invoked cleanly again. For example, for a composer, any input should be discarded.
    qDebug() << "cardPooled: " << doneMessage.reason();

    // TODO: Clean-up and release any resource the card might have used.
}

void CardUI::onInvoked(const bb::system::InvokeRequest& request)
{
    qDebug() << "onInvoked request:";

    bb::system::InvokeSource source = request.source();
    QString memo = QString::fromUtf8(request.data());

    qDebug() << "Source: (" << source.groupId() << "," << source.installId() << ")";
    qDebug() << "Target:" << request.target();
    qDebug() << "Action:" << request.action();
    qDebug() << "Mime:" << request.mimeType();
    qDebug() << "Url:" << request.uri().toString();
    qDebug() << "Data:" << memo;
    QString target = request.target();
    if (target.compare("in.ahamtech.Coterie.share") == 0) {
        QmlDocument *qml = QmlDocument::create("asset:///cards/share.qml").parent(this);

        // Make C++ UI part available to the qml.
        qml->setContextProperty("app", this);

        // Create root object for the UI
        AbstractPane *root = qml->createRootObject<AbstractPane>();

        Application::instance()->setScene(root);

    } else if (target.compare("in.ahamtech.Coterie.card") == 0) {
        QmlDocument *qml = QmlDocument::create("asset:///cards/chatview.qml").parent(this);
        QDeclarativePropertyMap *filepathname = new QDeclarativePropertyMap(this);
        filepathname->insert("media",
                QVariant(QString("file://" + QDir::homePath() + "/ahammedia/")));

        qml->setContextProperty("filepathname", filepathname);
        // Make C++ UI part available to the qml.
        qml->setContextProperty("app", this);

        // Create root object for the UI
        AbstractPane *root = qml->createRootObject<AbstractPane>();

        Application::instance()->setScene(root);

    }else if(target.compare("in.ahamtech.Coterie.shared") == 0){
        QmlDocument *qml = QmlDocument::create("asset:///cards/text.qml").parent(this);

        // Make C++ UI part available to the qml.
        qml->setContextProperty("app", this);

        // Create root object for the UI
        AbstractPane *root = qml->createRootObject<AbstractPane>();

        Application::instance()->setScene(root);
    }
    emit memoChanged(request.uri().toString(), request.mimeType());
    emit textChanged(memo);
}

QVariant CardUI::loadChat(QString id, qint64 count)
{
    return db->executeQuery(
            QString("select * from messaging where channel = \"%1\" order by ts asc").arg(id));
}
void CardUI::settingsChanged(const QString& changed_string)
{
    qDebug() << "Card changed string" << changed_string;
    QSettings settings(m_author, m_appName);
    if( settings.value("type").toString() == "new"){
        Q_EMIT incomingMessage("new",settings.value("channel").toString());
    }
    if(settings.value("type").toString() == "delete"){
        Q_EMIT incomingMessage("delete",settings.value("ts").toString());
    }
    if(settings.value("type").toString() == "update"){
            Q_EMIT incomingMessage("update",settings.value("ts").toString());
        }
    settings.remove("type");
    settings.remove("channel");
    settings.remove("ts");
}
QVariant CardUI::getLastChat(QString id)
{
    return db->executeQuery(QString("select * from messaging where channel = \"%1\"").arg(id));
}
bool CardUI::checkNotify(QString ntfy)
{
    return (db->getTableSizeByQuery(
            QString("select count(*) from notifications where channel = \"%1\"").arg(ntfy)) >= 1);

}
void CardUI::insertNotify(QString ntfy)
{
    QVariantMap map;
    map["channel"] = ntfy;
    db->insertQuery(QString("insert into notifications(channel)values(:channel)"), map);
}
QVariant CardUI::getActiveAccount()
{
    return db->executeQuery("select * from settings where type = 'primary'");
}
QVariant CardUI::getChannels()
{
    return db->executeQuery("select * from channels");
}
void CardUI::uploadFile(QString filePath, QString token, QString channel, QString title)
{
    uploader->uploadFile(filePath, token, channel, title);
}
bool CardUI::checkMuteChannels(QString channelid)
{
    return db->getTableSizeByQuery(
            QString("select count(*) from mutechannels where channelid = \"%1\"").arg(channelid))
            > 0;
}
void CardUI::insertMuteChannels(QString channelid)
{
    db->insertQuery(QString("insert into mutechannels(channelid)values(\"%1\")").arg(channelid),
            QVariantMap());
}
void CardUI::deleteMuteChannels(QString channelid)
{
    db->insertQuery(QString("delete from mutechannels where channelid = \"%1\"").arg(channelid),
            QVariantMap());

}
QVariant CardUI::getChannelById(QString channelid)
{
    return db->executeQuery(QString("select * from channels where id = \"%1\"").arg(channelid));
}
QVariant CardUI::getMessagesByTs(QString id, QString ts)
{
    return db->executeQuery(
            QString("select * from messaging where channel = \"%1\" and ts > \"%2\" ").arg(id, ts));
}
QVariant CardUI::getTs(QString ts)
{
    return db->executeQuery(
            QString("select * from messaging where ts  = \"%1\" ").arg(ts));
}
bool CardUI::checkChannelHistory(QString id)
{
    return db->getTableSizeByQuery(
            QString("select count(*) from channels where history = \"true\" and id = \"%1\"").arg(
                    id)) > 0;
}
void CardUI::setChannelHistory(QString id)
{
    QVariantMap map;
    map["id"] = id;
    db->insertQuery(QString("update channels set history = \"true\" where id = :id"), map);

}
void CardUI::deleteHistory(QString id){
    QVariantMap map;
        map["channel"] = id;
        db->insertQuery(QString("delete from messaging where channel = :channel"), map);
}
void CardUI::inserMessaging(QVariantMap map)
{
    db->insertQuery("insert into messaging(channel,user,text,ts,type, id)values(:channel,:user,:text,:ts,:type, :id)", map);

}
QVariant CardUI::getAppSettingsByType(QString type)
{
    return db->executeQuery(QString("select * from appsettings where type = \"%1\"").arg(type));
}
void CardUI::copyToClipBoard(QString text)
{
    bb::system::Clipboard board;
    board.insert("text/plain", text.toUtf8());
}
QString CardUI::getLocale()
{
    return QLocale().name();
}
void CardUI::onRequestCall()
{
    QObject::connect(dm->reply, SIGNAL(readyRead()), this, SLOT(readReady()));
    QObject::connect(dm->reply, SIGNAL(downloadProgress(qint64,qint64)), this,
            SLOT(downloadProgress(qint64,qint64)));
    QObject::connect(dm->reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
            SLOT(error(QNetworkReply::NetworkError)));
    QObject::connect(dm->reply, SIGNAL(finished()), this, SLOT(finished()));
}
void CardUI::downloadFiles(QString url_string, QString filename)
{
    qDebug() << url_string << filename;
    dm->append(QUrl(url_string), filename);
}
void CardUI::readReady()
{
    if (QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender())) {

        QString filename = reply->request().attribute(QNetworkRequest::User).toString();
        if (dm->downloads.contains(filename)) {
            QFile *file = dm->downloads.value(filename);
            file->write(reply->readAll());
        }
    }

}
void CardUI::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << "bytes received" << bytesReceived << "/" << bytesTotal;
    Q_EMIT attachmentProgres(bytesReceived, bytesTotal);
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);
}

void CardUI::error(QNetworkReply::NetworkError code)
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    qDebug() << "replyHeaders in eroor" << reply->rawHeaderList();
    qDebug() << "this is the error code" << code;
    Q_UNUSED(code);
    if (QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender())) {
        qErrnoWarning("Error!");

        QString filename = reply->request().attribute(QNetworkRequest::User).toString();

        dm->remove(filename);
        reply->deleteLater();
    }
}

void CardUI::finished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirect.isValid()) {
        QString filename =
                reply->request().attribute(QNetworkRequest::User).toString().split("/").last();
        if (dm->downloads.contains(filename))
            dm->remove(filename);
        QString reg = redirect.toString().split("wp.com\/").last();
        dm->append(QUrl(reg), filename);
    } else {
        if (reply) {
            QString filename = reply->request().attribute(QNetworkRequest::User).toString();
            dm->remove(filename);
            Q_EMIT attachmentFinish(reply->url().toString());
            reply->deleteLater();
            dm->nextDownload();
        }
//        Q_EMIT avatarDownloadFinish();
    }

}
