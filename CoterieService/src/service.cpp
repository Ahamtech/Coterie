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

#include "service.hpp"

#include <bb/Application>
#include <bb/platform/Notification>
#include <bb/platform/NotificationDefaultApplicationSettings>
#include <bb/system/InvokeManager>
#include <QtSql/QSqlQuery>
#include <bb/data/JsonDataAccess>
#include <QTimer>
#include <qdebug.h>
#include <qt4/Qt/qregexp.h>

using namespace bb::platform;
using namespace bb::data;
using namespace bb::system;
const QString Service::m_author = "Anwesh"; // for creating settings
const QString Service::m_appName = "Slack"; // for creating settings

Service::Service() :

        QObject(), m_notify(new Notification(this)), m_invokeManager(new InvokeManager(this)), stream(
                new Stream(this)), db(new Database(this)), account(NULL), _hubCache(NULL), _udsUtil(
        NULL), _settings(NULL), _itemCounter(0), cache(), users(), channels(), lastMessage(), settingsWatcher(
                new QFileSystemWatcher(this)), timer(new QTimer(this)), linkcheck(new QTimer(this))

{
    connect(timer, SIGNAL(timeout()), this, SLOT(reConnect()));
    connect(linkcheck, SIGNAL(timeout()), this, SLOT(checkConnection()));

    QSettings settings("Anwesh", "Primary");
    settings.setValue("myteam", "ahamtech");
    settings.sync();
    _settings = new QSettings("Anwesh", "Slack");
    settingsWatcher->addPath(settings.fileName());
    connect(settingsWatcher, SIGNAL(fileChanged(const QString&)), this,SLOT(settingsChanged(const QString&)));

    QFile(_settings->fileName());

    // Watcher for changes in the settings file.

    QObject::connect(stream, SIGNAL(parseRTM(QByteArray)), this, SLOT(parsingRTMResponse(QByteArray)));

    m_invokeManager->connect(m_invokeManager, SIGNAL(invoked(const bb::system::InvokeRequest&)), this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));

    QObject::connect(&m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
    QObject::connect(&m_webSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    QObject::connect(&m_webSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    QObject::connect(&m_webSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(onTextMessageReceived(QString)));

}

void Service::handleInvoke(const bb::system::InvokeRequest & request)
{

    if (request.action().compare("bb.action.system.STARTED") == 0 || request.action().compare("bb.action.RESTART") == 0 ) {
        qDebug() << "Primary is good one started";
        insertLogger("system started");
        timerStart();
        linkcheck->start(300000);
    }
}

void Service::initializeHub()
{
    qDebug() << "HeadlessHubIntegration: initialize: " << (_udsUtil != NULL);

    _initMutex.lock();

    // initialize UDS
    if (!_udsUtil) {
        _udsUtil = new UDSUtil(QString("Slack"), QString("hubassets"));
    }

    if (!_udsUtil->initialized()) {
        _udsUtil->initialize();
    }

    if (_udsUtil->initialized() && _udsUtil->registered()) {
        if (!_settings) {
            _settings = new QSettings("Anwesh", "SlackHub");
            qDebug() << "QSettings are getting updated";
        }
        if (!_hubCache) {
            _hubCache = new HubCache(_settings);
        }
        if (!account) {
            account = new HubIntegration(_udsUtil, _hubCache);
        }

        qDebug() << "HeadlessHubIntegration: initialize: initialized " << (_udsUtil != NULL);
    }
    _initMutex.unlock();
}

void Service::getPrimaryAccount()
{
    insertLogger("fetching primary account");
    QSqlQuery query = db->executeSqlQuery("select * from settings where type = 'primary'");
    QVariantMap acc_details;
    while (query.next()) {
        acc_details["name"] = query.value(2).toString();
        acc_details["key"] = query.value(1).toString();
    }
    if (acc_details["key"].toString() != NULL) {
        requestRTM(acc_details["key"].toString());
    }
}

void Service::requestRTM(QString url)
{

    QString rtm = "https://slack.com/api/rtm.start?token=";
    rtm.append(url);
    insertLogger(rtm);
    qDebug() << " Headless WS" << rtm;
    stream->stream_url(rtm);
}

void Service::onRequestRTMError(QString error)
{
    qDebug() << error;
    timer->start(10000);
}

void Service::timerStop()
{
    if (timer->isActive()){
        insertLogger(QString("Timer Stoped"));
        qDebug() << "Timer Stopped";
    }
    timer->stop();
}
void Service::timerStart()
{
    qDebug() << "Timer Started";
    timer->start(20000);
}

void Service::connectSocket(QString ipAddress)
{
    qDebug() << "Headless connecting to websocket" << ipAddress;
    m_webSocket.open(QUrl(ipAddress));
}

void Service::reConnect()
{
//    qDebug() << "connecting internnet";
    getPrimaryAccount();
}
void Service::checkConnection()
{

    qDebug() << "timer started to linkchecker";
    qDebug() << m_webSocket.state() ;
    if (!timer->isActive()  && m_webSocket.state()  == QAbstractSocket::ClosingState ) {
        timer->start(20000);
    }
}

void Service::parsingRTMResponse(QByteArray buffer)
{
    qDebug() << "parsingRTMsuccess";
    timerStop();
    initializeHub();
    users.clear();
    channels.clear();
    deleteChannels();
    cache.clear();
    bb::data::JsonDataAccess ja;
    const QVariant data = ja.loadFromBuffer(buffer);
    cache = data.toMap();
    QVariantList itemslist = _hubCache->items();
    qDebug() << "Items List" << itemslist.size();
    account->updateAccount("desc", cache["self"].toMap()["name"].toString());
    self["name"] = cache["self"].toMap()["name"].toString();
    self["id"] = cache["self"].toMap()["id"].toString();

    QList<QVariant> _channels = cache["channels"].toList();
    QList<QVariant> bots = cache["bots"].toList();
    QList<QVariant> groups = cache["groups"].toList();
    QList<QVariant> ims = cache["ims"].toList();
    QList<QVariant> _users = cache["users"].toList();

    foreach(QVariant user,_users){
    QVariantMap map = user.toMap();
    if(!map["deleted"].toBool()) {
        insertChannels(map["id"].toString(), map["profile"].toMap()["real_name"].toString(),"users");
        users.insert(map["id"].toString(),map["profile"].toMap()["real_name"].toString());
    }
}
    foreach(QVariant bot,bots){
    QVariantMap map = bot.toMap();

    if(!map["deleted"].toBool()) {
        insertChannels(map["id"].toString(), map["name"].toString(),"bots");
        users.insert(map["id"].toString(),map["name"].toString());
    }
}
    foreach(QVariant channel,_channels){
    QVariantMap map = channel.toMap();
    if(!map["is_archived"].toBool()) {
        QString nam = map["name"].toString();
        nam.prepend(QString("#"));
        channels.insert(map["id"].toString(),nam);
        bool check = false;
        qint64 sync;
        insertChannels(map["id"].toString(),nam,"channel");
        foreach(QVariant data,itemslist) {

            if(data.toMap()["syncId"].toString() == map["id"].toString()) {
                check = true;
                sync = data.toMap()["sourceId"].toLongLong();
            }
        }
        if(check == true) {
            qDebug() << nam << ": upadting" << data.toMap();
            if(map["latest"].toMap().size() > 0 )
                map["timestamp"] = timeSplit(map["latest"].toMap()["ts"].toString());
//            else
//                map['timestamp'] = qString("0");
            map["syncId"] = map["id"].toString();
            map["description"] = map["latest"].toMap()["text"].toString().mid(0,50);
            map["name"] = channels[map["id"].toString()];
            map["readCount"] = map["unread_count"].toInt() ==0?1:0;
            account->updateHubItem(qint64(1), sync, map, false);
        }
        else {
            qDebug() << nam << ": Insert" << data.toMap();
            account->addHubItem(qint64(1), map , nam ,map["latest"].toMap()["text"].toString(),timeSplit(map["latest"].toMap()["ts"].toString()),map["id"].toString(),"","",false);
        }
    }
}
    foreach(QVariant group,groups){
    QVariantMap map = group.toMap();
    if(!map["is_archived"].toBool()) {
        QString nam = map["name"].toString();
        nam.prepend(QString("~"));
        channels.insert(map["id"].toString(),nam);
        bool check = false;
        qint64 sync;
        insertChannels(map["id"].toString(),nam,"group");
        foreach(QVariant data,itemslist) {

            if(data.toMap()["syncId"].toString() == map["id"].toString()) {
                check = true;
                sync = data.toMap()["sourceId"].toLongLong();
            }
        }
        if(check == true) {
            qDebug() << nam << ": upadting" << data.toMap();
            if(map["latest"].toMap().size() > 0 )
                map["timestamp"] = timeSplit(map["latest"].toMap()["ts"].toString().mid(0,50));
//            else
//                map['timestamp'] =  qString("0");
            map["syncId"] = map["id"].toString();
            map["description"] = map["latest"].toMap()["text"].toString();
            map["name"] = channels[map["id"].toString()];
            account->updateHubItem(qint64(1), sync, map, false);
        }
        else
        account->addHubItem(qint64(1),map,nam,map["latest"].toMap()["text"].toString(),timeSplit(map["latest"].toMap()["ts"].toString()),map["id"].toString(),"","",false);

    }
}
    foreach(QVariant im,ims){
    QVariantMap map = im.toMap();
    QString mapuser = map["id"].toString();
    qDebug() << "User Check "<<users[mapuser];
    if(map["is_open"].toBool()  && !users[mapuser].toBool() ) {
    QString nam = users[map["user"].toString()].toString();
    nam.prepend(QString("@"));
    channels.insert(map["id"].toString(),nam);
    bool check = false;
    qint64 sync;
    insertChannels(map["id"].toString(),nam,"ims");
    foreach(QVariant data,itemslist) {

        if(data.toMap()["syncId"].toString() == map["id"].toString()) {
            check = true;
            sync = data.toMap()["sourceId"].toLongLong();
            qDebug() << nam;
            qDebug() << data.toMap()["syncId"].toString() << map["id"].toString();
        }
    }
    if(check == true) {
        qDebug() << nam << ": upadting" << data.toMap();
        map["timestamp"] = timeSplit(map["latest"].toMap()["ts"].toString());
        map["syncId"] = map["id"].toString();
        map["description"] = map["latest"].toMap()["text"].toString().mid(0,50);
        map["name"] = channels[map["id"].toString()];
        account->updateHubItem(qint64(1), sync, map, false);
    }
    else
    account->addHubItem(qint64(1),map, nam,map["latest"].toMap()["text"].toString().mid(0,50),timeSplit(map["latest"].toMap()["ts"].toString()),map["id"].toString(),"","",false);
    }
    }
    qDebug() << channels.size() << users.size();
    markHubItemsRead();
    connectSocket(cache["url"].toString());
//    }
//    reply->deleteLater();
}

void Service::displaySslErrors(const QList<QSslError>& errors)
{
    for (int i = 0, sz = errors.size(); i < sz; i++) {
        QString errorString = errors.at(i).errorString();
        qDebug() << errorString;
    }
}

void Service::socketConnected()
{
    qDebug() << "hub connecrtds ";
}
void Service::deleteMessaging(QString ts)
{
    db->insertQuery(QString("delete from messaging where ts = \"%1\"").arg(ts), QVariantMap());
}
void Service::updateMessaging(QString text, QString ts)
{
    db->insertQuery(QString("update messaging set text = \"%1\"where ts = \"%2\"").arg(text, ts),
            QVariantMap());
}
void Service::onTextMessageReceived(QString message)
{
    bb::data::JsonDataAccess ja;
    QVariant data = ja.loadFromBuffer(message);
    QVariantMap message_info = data.toMap();
    QSettings settings(m_author, m_appName);

    qDebug() << message_info;
    if (message_info["type"].toString() == "message") {

        if (message_info["subtype"].toString() == "message_deleted") {
            deleteMsgByTs(message_info["deleted_ts"].toString());
            settings.setValue("type", "delete");
            settings.setValue("ts", message_info["deleted_ts"].toString());
        } else if (message_info["subtype"].toString() == "message_changed") {
            updateMessaging(message_info["message"].toMap()["text"].toString(),
                    message_info["message"].toMap()["ts"].toString());
            settings.setValue("type", "update");
            settings.setValue("ts", message_info["message"].toMap()["ts"].toString());
        }

        else {
            QVariantMap map;
            map["ts"] = message_info["ts"].toString();
            map["text"] = message_info["text"].toString();
            map["user"] = message_info["user"].toString();
            map["channel"] = message_info["channel"].toString();
            map["type"] = "";
            map["id"] = "";
            if (message_info.contains("bot_id"))
                map["user"] = message_info["bot_id"].toString();
            if (message_info.contains("subtype")) {
                qDebug() << " subtype eneted ";
                if (message_info["subtype"].toString() == "file_comment") {
                    QString mess = tr("Commented on ") + " -- "
                            + message_info["file"].toMap()["name"].toString() + "\n";
                    mess.append(message_info["comment"].toMap()["comment"].toString());
                    map["text"] = mess;
                    map["user"] = message_info["comment"].toMap()["user"].toString();
                    map["type"] = "file";
                    map["id"] = message_info["file"].toMap()["id"].toString();
                    qDebug() << map["text"].toString();

                } else if (message_info["subtype"].toString() == "file_share") {
                    map["text"] = tr("Shared") + " -- "
                            + message_info["file"].toMap()["name"].toString() + "\n";
                    map["user"] = message_info["user"].toString();
                    map["type"] = "file";
                    map["id"] = message_info["file"].toMap()["id"].toString();

                }else if (message_info["subtype"].toString() == "bot_message") {
                    map["text"] = message_info["attachments"].toList().first().toMap()["fallback"].toString() + "\n";
                    map["user"] = message_info["bot_id"].toString();

                }

                else if (message_info["subtype"].toString() == "pinned_item") {
                    if( message_info["item_type"].toString() == "F" ){
                        map["text"] = tr("Pinned Item ") + " -- "
                                                    + message_info["item"].toMap()["name"].toString() + "\n";
                    }
                    else{
                        map["text"] = "Pinned Item -- "
                                                    + message_info["attachments"].toList().first().toMap()["fallback"].toString() + "\n";
                    }
                    map["user"] = message_info["user"].toString();
                    map["type"] = "pin";
                    map["id"] = "";
                }else{
                    QRegExp found("<@([^>]+)>");
                                QString m_string = message_info["text"].toString();
                                int pos = found.indexIn(m_string);
                                int match_length = found.matchedLength();
                                if( pos >= 0 ){
                                    QVariantList regresp = getChannelById(found.cap(1)).toList();
                                    qDebug() << "reso dat " << regresp;
                                    m_string.replace(pos,match_length,regresp.first().toMap()["name"].toString().prepend("@"));
                                    map["text"] = m_string;
                                }
                }
            }

            db->insertQuery(
                    QString(
                            "insert into messaging(channel,user,text,ts,type,id)values(:channel,:user,:text,:ts,:type,:id)"),
                    map);
            settings.setValue("type", "new");
            settings.setValue("channel", message_info["channel"].toString());

            QVariant acc = account->getHubItemBySync(qint64(1), message_info["channel"].toString());
            QVariantMap map2 = acc.toMap();
            QString des = map["text"].toString();
            if (message_info.contains("user"))
                des.prepend(QString(" ")).prepend(
                        (users[message_info["user"].toString()].toString()));
            else
                des.prepend(QString(" ")).prepend(
                        (users[message_info["username"].toString()].toString()));

            message_info["description"] = des.mid(0,50);

            bool notify = true;
            notify = message_info["user"].toString() == self["id"] ? false : "";
            notify = notify ? !checkMuteChannels(message_info["channel"].toString()) : notify;

            message_info["readCount"] = message_info["user"].toString() == self["id"] ? 1 : 0;
            message_info["timestamp"] = timeSplit(message_info["ts"].toString());
            message_info["syncId"] = message_info["channel"].toString();
            message_info["name"] = channels[message_info["channel"].toString()];

            account->updateHubItem(qint64(1), map2["sourceId"].toLongLong(), message_info, notify);
        }
    }
    //Mark an Private Message
    else if (message_info["type"].toString() == "im_marked") {
        QVariant acc = account->getHubItemBySync(qint64(1), message_info["channel"].toString());
        QVariantMap map2 = acc.toMap();
        markHubItemRead(map2);
    }
    //Mark an Channel and Group
    else if (message_info["type"].toString() == "channel_marked") {
        QVariant acc = account->getHubItemBySync(qint64(1), message_info["channel"].toString());
        QVariantMap map2 = acc.toMap();
        markHubItemRead(map2);
    } else if (message_info["type"].toString() == "group_marked") {
        QVariant acc = account->getHubItemBySync(qint64(1), message_info["channel"].toString());
        QVariantMap map2 = acc.toMap();
        markHubItemRead(map2);
    }
    //Group Joined
    else if (message_info["type"].toString() == "group_joined") {
        QVariantMap map = message_info["channel"].toMap();
        QString nam = map["name"].toString();
        nam.prepend("~");
        channels[map["id"].toString()] = nam;
        account->addHubItem(qint64(1), map, nam, map["latest"].toMap()["text"].toString(),
                timeSplit(map["latest"].toMap()["ts"].toString()), map["id"].toString(), "", "",
                false);

    }
    //Group Left or Group Closed
    else if (message_info["type"].toString() == "group_left"
            || message_info["type"].toString() == "group_close") {
        QVariant acc = account->getHubItemBySync(qint64(1), message_info["channel"].toString());
        QVariantMap map = acc.toMap();
        account->removeHubItem(qint64(1), map["sourceId"].toLongLong());
    }
    //Channel Joined
    else if (message_info["type"].toString() == "channel_joined") {
        QVariantMap map = message_info["channel"].toMap();
        QString nam = map["name"].toString();
        nam.prepend("#");
        channels[map["id"].toString()] = nam;
        account->addHubItem(qint64(1), map, nam, map["latest"].toMap()["text"].toString(),
                timeSplit(map["latest"].toMap()["ts"].toString()), map["id"].toString(), "", "",
                false);

    }
    //Channel Left or Channel Deleted
    else if (message_info["type"].toString() == "channel_left"
            || message_info["type"].toString() == "channel_deleted") {
        QVariant acc = account->getHubItemBySync(qint64(1), message_info["channel"].toString());
        QVariantMap map = acc.toMap();
        account->removeHubItem(qint64(1), map["sourceId"].toLongLong());
    }
    //Bot Added or Updated
    else if (message_info["type"].toString() == "bot_added"
            || message_info["type"].toString() == "bot_changed") {
        QVariantMap map = message_info["bot"].toMap();
        QString nam = map["name"].toString();
        nam.prepend("#");
        channels[map["id"].toString()] = nam;
    } else if (message_info["type"].toString() == "im_open") {

    } else {

    }

}
void Service::socketDisconnected()
{
    qDebug() << "dis connected ";
    timerStart();
}

void Service::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
        case QAbstractSocket::UnconnectedState:
            qDebug() << " Headless Unconnecte state";
            break;
        case QAbstractSocket::HostLookupState:
            qDebug() << "Headless  Host look ups tate";
            break;
        case QAbstractSocket::ConnectingState:
            qDebug() << "Headless  Connecting ";
            break;
        case QAbstractSocket::ConnectedState:
            timerStop();
            qDebug() << "Headless Connected";
            insertLogger("WS connected ");
            break;
        case QAbstractSocket::BoundState:
            qDebug() << "Headless Bound Sate";
            break;
        case QAbstractSocket::ClosingState:
            timerStart();
            qDebug() << "Headless Closing Sate";
            break;
        case QAbstractSocket::ListeningState:
            qDebug() << "Headless Listing ";
            break;
        default:
            qDebug() << "Headless Unconn";
            break;
    }
}

qlonglong Service::timeSplit(QString ts)
{
    return (ts.split(".")[0].toLongLong() * 1000);
}

void Service::markHubItemsRead()
{
    QVariantList h_list = account->items();
    Q_FOREACH(QVariant data,h_list){
        QVariantMap map = data.value<QVariantMap>();
        qint64 itemdid = map.value("sourceId").toLongLong();
        account->markHubItemRead(qint64(1), itemdid);
    }
}

void Service::markHubItemRead(QVariantMap itemProperties)
{
//    qDebug() << "HeadlessHubIntegration::markHubItemRead: item: " << itemProperties;
//
//    qDebug() << "HeadlessHubIntegration::markHubItemRead: item src Id: "
//            << itemProperties["sourceId"].toString();

    qint64 itemId;
    if (itemProperties["sourceId"].toString().length() > 0) {
        itemId = itemProperties["sourceId"].toLongLong();
    } else if (itemProperties["messageid"].toString().length() > 0) {
        itemId = itemProperties["messageid"].toLongLong();
    }
    account->markHubItemRead(qint64(1), itemId);
}

void Service::settingsChanged(const QString& changed_string)
{
    QSettings settings(m_author, "Primary");
    if (settings.value("primary") == true) {
        qDebug() << "Primary acitveted he;";
        insertLogger("qsettings trigered from UI");
        getPrimaryAccount();

    }
    qDebug() << "Headless Card QSettings" << changed_string;

}

bool Service::checkNotify(QString ntfy)
{
    return (db->getTableSizeByQuery(
            QString("select count(*) from notifications where channel = \"%1\"").arg(ntfy)) >= 1);

}
void Service::insertLogger(QString text)
{
    QVariantMap map;
    map["time_stamp"] = QDateTime::currentDateTime().toLocalTime();
    map["text"] = text;
    db->insertQuery(QString("insert into logger(timestamp,text)values(:time_stamp,:text)"), map);
}
void Service::insertChannels(QString id, QString channelName, QString type)
{
    QVariantMap map;
    map["id"] = id;
    map["name"] = channelName;
    map["type"] = type;
    map["history"] = false;
    db->insertQuery("insert into channels(id,name,type,history)values(:id,:name,:type,:history)",
            map);
}
void Service::deleteChannels()
{
    db->deleteTable("channels");
}
void Service::deleteChannelById(QString id)
{
    db->insertQuery(QString("delete from channels where id = \"%1\"").arg(id), QVariantMap());
}
bool Service::checkMuteChannels(QString channelid)
{
    return db->getTableSizeByQuery(
            QString("select count(*) from mutechannels where channelid = \"%1\"").arg(channelid))
            > 0;
}
void Service::deleteMsgByTs(QString ts)
{
    db->insertQuery(QString("delete from messaging where ts = \"%1\"").arg(ts), QVariantMap());
}
void Service::updateMsgs(QString text, QString ts)
{
    QVariantMap map;

    map["ts"] = ts;
    map["text"] = text;

    db->insertQuery("update  messaging set text = :text where ts = :ts", map);
}

QVariant Service::getChannelById(QString channelid)
{
    return db->executeQuery(QString("select * from channels where id = \"%1\"").arg(channelid));
}
