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

#ifndef ApplicationUI_HPP_
#define ApplicationUI_HPP_
#include <QtNetwork/QTcpSocket>
#include "database.hpp"
#include <QTimer>
#include <QObject>
#include <QtNetwork/QAbstractSocket>
#include <QtNetwork/QSslSocket>
#include "socket/qwebsocket.h"
#include "DownloadManager.hpp"
#include <QFileSystemWatcher>
#include "Uploader.hpp"
#include "Stream.h"
class DownloadManager;
class QFileSystemWatcher;
class Uploader;
class QTimer;
namespace bb
{
    namespace cascades
    {
        class LocaleHandler;
    }
}

class QTranslator;

/*!
 * @brief Application UI object
 *
 * Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class ApplicationUI: public QObject
{
Q_OBJECT
public:
    ApplicationUI();
    virtual ~ApplicationUI()
    {
    }
    Q_INVOKABLE
    void initConnection(QString);Q_INVOKABLE
    QString getSettingsByType(QString);Q_INVOKABLE
    void updateSettings(QString, QString, QString);Q_INVOKABLE
    void insertAppSettings(QString, QString);Q_INVOKABLE
    void insertSettings(QString, QString);Q_INVOKABLE
    void logincomplete();Q_INVOKABLE
    QVariant getActiveAccount();Q_INVOKABLE
    int settingsCountByType(QString);Q_INVOKABLE
    void connectSocket(QString);Q_INVOKABLE
    QVariant getSettings();Q_INVOKABLE
    void logOut(QString);Q_INVOKABLE
    void executeStreamUrl(QString);Q_INVOKABLE
    QVariant getPrimaryAccount();Q_INVOKABLE
    void disconnectSocket();Q_INVOKABLE
    void downloadFiles(QString, QString);Q_INVOKABLE
    bool isNetworkAvailable();Q_INVOKABLE
    void activePrimary();
    Q_INVOKABLE bool checkNotify(QString);
    Q_INVOKABLE void deleteMsgById(QString);
    Q_INVOKABLE void deleteMsgs();

    Q_INVOKABLE void uploadFile(QString,QString,QString,QString);
    Q_INVOKABLE QString getLocale();
    Q_INVOKABLE void insertMsgs(QString,QString,QString,QString,QString,QString);
    Q_INVOKABLE void deleteMsgByTs(QString);
    Q_INVOKABLE QVariant getMsgsById(QString);
    Q_INVOKABLE void  deleteMuteChannels(QString);
    Q_INVOKABLE void insertMuteChannels(QString);
    Q_INVOKABLE bool checkMuteChannels(QString);
    Q_INVOKABLE void copyToClipBoard(QString);
    Q_INVOKABLE void invokeBrowser(QString);
    Q_INVOKABLE void invokePictureView(QString);
    Q_INVOKABLE void inserMessaging(QVariantMap);
    Q_INVOKABLE QVariant getAppSettingsByType(QString);
    Q_INVOKABLE void updateMsgs(QString,QString);
    Q_INVOKABLE void invokeSystemSettings();
    Q_INVOKABLE void disconnectSocketForSwitchAccount();
    Q_INVOKABLE void headlessRestart();
    Q_INVOKABLE void headlessStop();



private Q_SLOTS:
    void onTextMessageReceived(QString);
    void readReady();
        void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void error(QNetworkReply::NetworkError code);
        void finished();

signals:
    void incomingMsg(QString);
    void reconnectass();
Q_SIGNALS:
    void attachmentProgres(qint64,qint64);
    void attachmentFinish(QString);
    void avatarDownloadFinish();
private slots:
    void reConnect();
    void onRequestCall();
    void onSystemLanguageChanged();
    void mainView();
    void sendMessage();
    void displayMessage(QString message);
    void socketConnected();
    void socketDisconnected();
    void socketStateChanged(QAbstractSocket::SocketState);
    void displaySslErrors(const QList<QSslError>& errors);
    void settingsChanged(const QString&);
    void slotUploadProgress(qint64 bytesSent, qint64 bytesTotal );
    void slotError(QNetworkReply::NetworkError error );
    void slotReadyRead();
    void slotFinished(QNetworkReply* reply);

private:
    static const QString m_author; // for creating settings
    static const QString m_appName; // for creating settings
    QFileSystemWatcher* settingsWatcher;
    Database *db;
    QTranslator* m_translator;
    bb::cascades::LocaleHandler* m_localeHandler;
    QWebSocket *m_webSocket;
    Stream* m_stream;
    DownloadManager *dm;
    Uploader *uploader;
    void timerStart();
    void timerStop();
    QTimer *timer;

};

#endif /* ApplicationUI_HPP_ */
