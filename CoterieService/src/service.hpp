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

#ifndef SERVICE_H_
#define SERVICE_H_

#include <QObject>
#include "socket/qwebsocket.h"
#include "socket/qwebsocketprotocol.h"
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QNetworkReply>
#include "socket/qwebsocket.h"
#include "Stream.h"
#include "database.hpp"
#include <QSettings>
#include <bb/pim/unified/unified_data_source.h>
#include "Hub/HubCache.hpp"
#include "Hub/UDSUtil.hpp"
#include "Hub/HubIntegration.hpp"
class Database;
class QNetworkReply;
class Stream;
class QWebSocket;

namespace bb
{

    namespace platform
    {
        class Notification;
    }
    namespace system
    {
        class InvokeManager;
        class InvokeRequest;
        class InvokeTargetReply;
        class ApplicationStartupMode;
    }
}

using bb::system::ApplicationStartupMode;
using bb::system::InvokeManager;
using bb::system::InvokeRequest;
using bb::system::InvokeTargetReply;
class Service: public QObject
{
    Q_OBJECT
public:
    Service();

//    Q_SLOT void onInvoked(const bb::system::InvokeRequest& request);
    virtual ~Service()
    {
    }

private Q_SLOTS:
    void parsingRTMResponse(QByteArray);

private slots:
    void handleInvoke(const bb::system::InvokeRequest &);
    void socketConnected();
    void onTextMessageReceived(QString);
    void socketDisconnected();
    void socketStateChanged(QAbstractSocket::SocketState);
    void displaySslErrors(const QList<QSslError>& errors);
    void reConnect();
    void checkConnection();
    void settingsChanged(const QString&);
    void insertChannels(QString,QString,QString);
    void deleteChannels();
    void deleteChannelById(QString);
    void updateMsgs(QString,QString);

private:
    bool checkMuteChannels(QString);
    QTimer *timer;
    QTimer *linkcheck;
    bool checkNotify(QString);
    void insertLogger(QString);
    static const QString m_author; // for creating settings
    static const QString m_appName; // for creating settings
    QFileSystemWatcher* settingsWatcher;
    void timerStart();
    void timerStop();
    QVariantMap self;
    HubCache* _hubCache;
    UDSUtil* _udsUtil;
    QSettings* _settings;
    HubIntegration* account;
    int _itemCounter;

    InvokeManager* _invokeManager;
    QMutex _initMutex;

    QWebSocket m_webSocket;
    void initializeHub();
    Database *db;
    bb::platform::Notification * m_notify;
    bb::system::InvokeManager * m_invokeManager;
    Stream *stream;
    void requestRTM(QString); //Request real time message
    void getPrimaryAccount();
    void markHubItemsRead();
    void connectSocket(QString);
    QVariant getChannelById(QString);
    void markHubItemRead(QVariantMap itemProperties);
    QVariantMap users;
    QVariantMap channels;

    QVariantMap lastMessage;
    QVariantMap cache;
    void onRequestRTMError(QString);
    qlonglong timeSplit(QString);
    void deleteMessaging(QString);
    void updateMessaging(QString,QString);
    void deleteMsgByTs(QString);

};

#endif /* SERVICE_H_ */
