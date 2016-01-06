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

#include "applicationui.hpp"
#include <QtCore/QLocale>
#include <bb/cascades/application.h>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/system/InvokeManager>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QtNetwork/QNetworkConfiguration>
#include <QtNetwork/QTcpSocket>
#include "socket/qwebsocket.h"
#include "socket/qwebsocketprotocol.h"

#include <QObject>
#include <QtSql/QSqlQuery>
#include <bb/system/Clipboard>

#include "Stream.h"
using namespace bb::cascades;
using namespace bb::system;
using namespace QWebSocketProtocol;
const QString ApplicationUI::m_author = "Anwesh"; // for creating settings
const QString ApplicationUI::m_appName = "Slack"; // for creating settings

ApplicationUI::ApplicationUI() :
        QObject(), m_translator(new QTranslator(this)), m_localeHandler(new LocaleHandler(this)), db(
                new Database(this)), m_stream(new Stream(this)), dm(new DownloadManager(this)), uploader(
                new Uploader(this)),m_webSocket(new QWebSocket("",V_Unknow,this)),timer(new QTimer(this))

{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    // prepare the localization
    if (!QObject::connect(m_localeHandler, SIGNAL(systemLanguageChanged()), this,
            SLOT(onSystemLanguageChanged()))) {
        // This is an abnormal situation! Something went wrong!
        // Add own code to recover here
        qWarning() << "Recovering from a failed connect()";
    }
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(reConnect()));
    QObject::connect(dm, SIGNAL(requestCall()), this, SLOT(onRequestCall()));

    QObject::connect(uploader, SIGNAL(signalReadyRead()), this, SLOT(slotReadyRead()));
    QObject::connect(uploader, SIGNAL(siganlError(QNetworkReply::NetworkError error )), this,
            SLOT(slotError(QNetworkReply::NetworkError error )));
    QObject::connect(uploader, SIGNAL(signalFinished(QNetworkReply* reply)), this,
            SLOT(slotFinished(QNetworkReply* reply)));
    QObject::connect(uploader, SIGNAL(signalUploadProgress(qint64 bytesSent, qint64 bytesTotal )),
            this, SIGNAL(attachmentProgres(qint64 bytesSent, qint64 bytesTotal )));

    QObject::connect(m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this,
            SLOT(socketStateChanged(QAbstractSocket::SocketState)));
    QObject::connect(m_webSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    QObject::connect(m_webSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(m_webSocket, SIGNAL(textMessageReceived(QString)), this,
            SLOT(onTextMessageReceived(QString)));
    // initial load
    onSystemLanguageChanged();

    // Create scene document from main.qml asset, the parent is set
    // to ensure the document gets destroyed properly at shut down.
    QDeclarativePropertyMap *filepathname = new QDeclarativePropertyMap(this);
    filepathname->insert("media", QVariant(QString("file://" + QDir::homePath() + "/ahammedia/")));

    QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
    qml->setContextProperty("filepathname", filepathname);
    // Make app available to the qml.
    qml->setContextProperty("app", this);

    // Create root object for the UI
    AbstractPane *root = qml->createRootObject<AbstractPane>();

    // Set created root object as the application scene
    Application::instance()->setScene(root);
//    executeStreamUrl("httpp");
}

void ApplicationUI::onSystemLanguageChanged()
{
    QCoreApplication::instance()->removeTranslator(m_translator);
    // Initiate, load and install the application translation files.
    QString locale_string = QLocale().name();
    QString file_name = QString("Coterie_%1").arg(locale_string);
    if (m_translator->load(file_name, "app/native/qm")) {
        QCoreApplication::instance()->installTranslator(m_translator);
    }
}

void ApplicationUI::initConnection(QString url)
{

}

void ApplicationUI::sendMessage()
{

}

void ApplicationUI::displayMessage(QString message)
{
//  ui->chatTextEdit->append(message);
    qDebug() << message << "message connection is connected";
}

void ApplicationUI::displaySslErrors(const QList<QSslError>& errors)
{
    for (int i = 0, sz = errors.size(); i < sz; i++) {
        QString errorString = errors.at(i).errorString();
        qDebug() << errorString;
    }
}

void ApplicationUI::connectSocket(QString ipAddress)
{
    m_webSocket->open(QUrl(ipAddress));

}

void ApplicationUI::disconnectSocket()
{
    m_webSocket->close(QWebSocketProtocol::CC_NORMAL, QString("Moving out"));

}
void ApplicationUI::disconnectSocketForSwitchAccount(){
    m_webSocket->flush();
    m_webSocket->abort();
    delete m_webSocket;
    m_webSocket = new QWebSocket("",QWebSocketProtocol::V_Unknow,this);
    QObject::connect(m_webSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this,
               SLOT(socketStateChanged(QAbstractSocket::SocketState)));
       QObject::connect(m_webSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
       QObject::connect(m_webSocket, SIGNAL(connected()), this, SLOT(socketConnected()));
       connect(m_webSocket, SIGNAL(textMessageReceived(QString)), this,
               SLOT(onTextMessageReceived(QString)));
}
void ApplicationUI::socketConnected()
{
    qDebug() << "conntected ";
    timerStop();
}

void ApplicationUI::socketDisconnected()
{
//    displayMessage(tr("DISCONNECTED"));
    qDebug() << "dis connected ";
}

void ApplicationUI::socketStateChanged(QAbstractSocket::SocketState socketState)
{
    switch (socketState) {
        case QAbstractSocket::UnconnectedState:
            qDebug() << "Unconnecte state";
            break;
        case QAbstractSocket::HostLookupState:
            qDebug() << "Host look ups tate";
            break;
        case QAbstractSocket::ConnectingState:
            qDebug() << "Connecting ";
            break;
        case QAbstractSocket::ConnectedState:
            qDebug() << "Connected";
            break;
        case QAbstractSocket::BoundState:
            qDebug() << "Bound Sate";
            break;
        case QAbstractSocket::ClosingState:
            qDebug() << "Closing Sate";
            timerStart();
            break;
        case QAbstractSocket::ListeningState:
            qDebug() << "Listing ";
            break;
        default:
            qDebug() << "Unconn";
            break;
    }
}

void ApplicationUI::onTextMessageReceived(QString s)
{
    Q_EMIT incomingMsg(s);
}
//void ApplicationUI::Login()
//{
//    QmlDocument *qml = QmlDocument::create("asset:///login.qml").parent(this);
//
//    // Make app available to the qml.
//    qml->setContextProperty("app", this);
//
//    // Create root object for the UI
//
//    // Set created root object as the application scene
//    Application::instance()->setScene(root);
//}
void ApplicationUI::logincomplete()
{
    mainView();
}
void ApplicationUI::mainView()
{

}

void ApplicationUI::insertSettings(QString type, QString value)
{

    db->insertQuery(
            QString("insert into settings(type,value) values(\"%1\",\"%2\")").arg(type, value),
            QVariantMap());

}
void ApplicationUI::updateSettings(QString value, QString name, QString active)
{
    db->insertQuery("update settings set active = 'false' where 1=1", QVariantMap());
    QVariantMap map;
    map["active"] = active;
    map["name"] = name;
    map["value"] = value;
    db->insertQuery(
            QString("update settings set account = :name , active = :active where value = :value"),
            map);
}
QVariant ApplicationUI::getActiveAccount()
{
    return db->executeQuery("select * from settings where active = 'true'");
}
int ApplicationUI::settingsCountByType(QString type)
{
    return db->getTableSizeByQuery(
            QString("select count(*) from settings where type = \"%1\"").arg(type));
}
QString ApplicationUI::getSettingsByType(QString type)
{
    QSqlQuery query = db->executeSqlQuery(
            QString("select value from settings where type = \"%1\"").arg(type));
    QString token;
    while (query.next()) {
        token = query.value(0).toString();
    }
    return token;
}
void ApplicationUI::executeStreamUrl(QString url)
{
    QString usr =
            "https://stream.flowdock.com/flows?api=6fce0ca5ab96deaf99dce45cb8ed1025&filter=ahamtech";
    qDebug() << usr;
    m_stream->stream_url(usr);
}
QVariant ApplicationUI::getSettings()
{
    return db->executeQuery("select * from settings");
}
void ApplicationUI::logOut(QString value)
{
    QVariantMap map;
    map["value"] = value;
    db->insertQuery(QString("delete from settings where value = :value"), map);
}
QVariant ApplicationUI::getPrimaryAccount()
{
    return db->executeQuery("select * from settings where type = \"primary\"");
}
bool ApplicationUI::isNetworkAvailable()
{
    QNetworkConfigurationManager netMgr;
    QList<QNetworkConfiguration> mNetList = netMgr.allConfigurations(QNetworkConfiguration::Active);

    return (mNetList.count() > 0 && netMgr.isOnline());
}

void ApplicationUI::activePrimary()
{
//    InvokeRequest request;
//    request.setTarget("in.ahamtech.SlackService");
//    request.setAction("in.ahamtech.SlackService.Primary");
    QSettings settings(m_author, "Primary");
    settings.setValue("primary", true);
//    m_invokeManager->invoke(request);
}

void ApplicationUI::downloadFiles(QString url_string, QString filename)
{
    dm->append(QUrl(url_string), filename);
}
void ApplicationUI::settingsChanged(const QString& changed_string)
{
    qDebug() << "changed string" << changed_string;
}
bool ApplicationUI::checkNotify(QString ntfy)
{
    return (db->getTableSizeByQuery(
            QString("select count(*) from notifications where channel = \"%1\"").arg(ntfy)) >= 1);

}
QString ApplicationUI::getLocale()
{
    return QLocale().name();
}

void ApplicationUI::slotUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
    qDebug() << "uploading start";
    qDebug() << bytesSent << "/" << bytesTotal;
}
void ApplicationUI::slotError(QNetworkReply::NetworkError error)
{
    qDebug() << "error in uploading" << error;
}
void ApplicationUI::uploadFile(QString filePath, QString token, QString channel, QString title)
{
    qDebug() << filePath << token;
    uploader->uploadFile(filePath, token, channel, title);
}
void ApplicationUI::slotFinished(QNetworkReply* reply)
{
    qDebug() << reply->readAll();
    qDebug() << "uploading is succcess";
    emit attachmentFinish(reply->url().toString());
    reply->deleteLater();
}
void ApplicationUI::slotReadyRead()
{

    qDebug() << "uploadindg is started url executions";
}

void ApplicationUI::insertAppSettings(QString type, QString value)
{
    QVariantMap map;
    map["value"] = value;
    map["type"] = type;
    int count = db->getTableSizeByQuery(
            QString("select count(*) from appsettings where type = \"%1\"").arg(type));
    if (count > 0) {
        db->insertQuery("update appsettings set value = :value where type = :type", map);

    } else {
        db->insertQuery(
                QString("insert into appsettings(type,value) values(\"%1\",\"%2\")").arg(type,
                        value), QVariantMap());

    }
}
QVariant ApplicationUI::getAppSettingsByType(QString type)
{
    return db->executeQuery(QString("select * from appsettings where type = \"%1\"").arg(type));
}
void ApplicationUI::insertMsgs(QString channel, QString user, QString text, QString ts,
        QString type, QString id)
{
    QVariantMap map;
    map["channel"] = channel;
    map["user"] = user;
    map["ts"] = ts;
    map["text"] = text;
    map["type"] = type;
    map["id"] = id;
    db->insertQuery(
            "insert into msgs(channel,user,text,ts,type,id)values(:channel,:user,:text,:ts,:type,:id)",
            map);
}
void ApplicationUI::updateMsgs(QString text, QString ts)
{
    QVariantMap map;

    map["ts"] = ts;
    map["text"] = text;

    db->insertQuery(
            "update  msgs set text = :text where ts = :ts",
            map);
}


void ApplicationUI::deleteMsgs()
{
    db->deleteTable("msgs");
}
void ApplicationUI::deleteMsgById(QString id)
{
    db->insertQuery(QString("delete from msgs where channel = \"%1\"").arg(id), QVariantMap());
}
void ApplicationUI::deleteMsgByTs(QString ts)
{
    db->insertQuery(QString("delete from msgs where ts = \"%1\"").arg(ts), QVariantMap());
}
QVariant ApplicationUI::getMsgsById(QString id)
{
    return db->executeQuery(
            QString(" select * from msgs where channel = \"%1\" order by ts asc").arg(id));
}
void ApplicationUI::inserMessaging(QVariantMap map)
{
    db->insertQuery("insert into msgs(channel,user,text,ts,type, id)values(:channel,:user,:text,:ts,:type, :id)", map);
}
void ApplicationUI::insertMuteChannels(QString channelid)
{
    db->insertQuery(QString("insert into mutechannels(channelid)values(\"%1\")").arg(channelid),
            QVariantMap());
}
void ApplicationUI::deleteMuteChannels(QString channelid)
{
    db->insertQuery(QString("delete from mutechannels where channelid = \"%1\"").arg(channelid),
            QVariantMap());

}
bool ApplicationUI::checkMuteChannels(QString channelid)
{
    return db->getTableSizeByQuery(
            QString("select count(*) from mutechannels where channelid = \"%1\"").arg(channelid))
            > 0;
}
void ApplicationUI::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << "bytes received" << bytesReceived << "/" << bytesTotal;
    Q_EMIT attachmentProgres(bytesReceived, bytesTotal);
    Q_UNUSED(bytesReceived);
    Q_UNUSED(bytesTotal);
}

void ApplicationUI::error(QNetworkReply::NetworkError code)
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

void ApplicationUI::finished()
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
        Q_EMIT avatarDownloadFinish();
    }

}
void ApplicationUI::readReady()
{
    if (QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender())) {

        QString filename = reply->request().attribute(QNetworkRequest::User).toString();
        if (dm->downloads.contains(filename)) {
            QFile *file = dm->downloads.value(filename);
            file->write(reply->readAll());
        }
    }

}
void ApplicationUI::onRequestCall()
{
    QObject::connect(dm->reply, SIGNAL(readyRead()), this, SLOT(readReady()));
    QObject::connect(dm->reply, SIGNAL(downloadProgress(qint64,qint64)), this,
            SLOT(downloadProgress(qint64,qint64)));
    QObject::connect(dm->reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
            SLOT(error(QNetworkReply::NetworkError)));
    QObject::connect(dm->reply, SIGNAL(finished()), this, SLOT(finished()));
}
void ApplicationUI::copyToClipBoard(QString text)
{
    bb::system::Clipboard board;
    board.insert("text/plain", text.toUtf8());
}
void ApplicationUI::invokePictureView(const QString uri) {
    InvokeRequest cardRequest;
        cardRequest.setUri(uri);
    cardRequest.setAction("bb.action.VIEW");
    cardRequest.setTarget("sys.pictures.card.previewer");
    InvokeManager *mInvokeManager = new InvokeManager(this);
    mInvokeManager->invoke(cardRequest);
    delete mInvokeManager;
}
void ApplicationUI::invokeBrowser(const QString uri) {
    InvokeRequest cardRequest;
        cardRequest.setUri(uri);
    cardRequest.setAction("bb.action.OPEN");
    cardRequest.setTarget("sys.browser");
    InvokeManager *mInvokeManager = new InvokeManager(this);
    mInvokeManager->invoke(cardRequest);
    delete mInvokeManager;
}
void ApplicationUI::timerStop(){
    if(timer->isActive())
        timer->stop();
}
void ApplicationUI::timerStart(){
    timer->start(20000);
}
void ApplicationUI::reConnect(){
    qDebug() << "UI reconnectonig " ;
    Q_EMIT reconnectass();
}
void ApplicationUI::invokeSystemSettings(){
    InvokeRequest cardRequest;
            cardRequest.setUri("settings://notification/application?id=in.ahamtech.coterie");
        cardRequest.setAction("bb.action.OPEN");
        cardRequest.setTarget("sys.settings.card");
        InvokeManager *mInvokeManager = new InvokeManager(this);
        mInvokeManager->invoke(cardRequest);
        delete mInvokeManager;

}

void ApplicationUI::headlessStop(){
    InvokeRequest cardRequest;
            cardRequest.setUri("devicemonitor://cpu/application?id=in.ahamtech.coterie");
            cardRequest.setAction(" bb.action.VIEW");
            cardRequest.setTarget("sys.SysMon.app");
            InvokeManager *mInvokeManager = new InvokeManager(this);
            mInvokeManager->invoke(cardRequest);
            delete mInvokeManager;
}

void ApplicationUI::headlessRestart(){
    InvokeRequest cardRequest;
    cardRequest.setAction("bb.action.RESTART");
    cardRequest.setTarget("in.ahamtech.CoterieService");
    InvokeManager *mInvokeManager = new InvokeManager(this);
    mInvokeManager->invoke(cardRequest);
    delete mInvokeManager;
}
