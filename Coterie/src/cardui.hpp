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

#ifndef CardUI_HPP_
#define CardUI_HPP_
#include <QFileSystemWatcher>
#include "Uploader.hpp"
#include "database.hpp"
#include "DownloadManager.hpp"

#include "applicationuibase.hpp";
#include <bb/system/InvokeManager>
class QFileSystemWatcher;
class Database;
class DownloadManager;

class Uploader;
namespace bb
{
    namespace system
    {
        class InvokeManager;
        class InvokeRequest;
    }
}

class CardUI: public ApplicationUIBase
{
Q_OBJECT
public:
    CardUI(bb::system::InvokeManager* invokeManager);
    CardUI();
    virtual ~CardUI()
    {
    }

    Q_INVOKABLE
    QVariant loadChat(QString id, qint64 count);Q_INVOKABLE
    QVariant getLastChat(QString);Q_INVOKABLE
    bool checkNotify(QString);Q_INVOKABLE
    void insertNotify(QString);Q_INVOKABLE
    QVariant getChannels();Q_INVOKABLE
    QVariant getActiveAccount();Q_INVOKABLE
    void uploadFile(QString filePath, QString token, QString channel, QString title);
    Q_INVOKABLE bool checkMuteChannels(QString);
    Q_INVOKABLE void deleteMuteChannels(QString);
    Q_INVOKABLE void insertMuteChannels(QString);
    Q_INVOKABLE QVariant getMessagesByTs(QString,QString);
    Q_INVOKABLE void setChannelHistory(QString);
    Q_INVOKABLE bool checkChannelHistory(QString);
    Q_INVOKABLE void inserMessaging(QVariantMap);
    Q_INVOKABLE QVariant getChannelById(QString);
    Q_INVOKABLE void copyToClipBoard(QString);
    Q_INVOKABLE QVariant getAppSettingsByType(QString);
    Q_INVOKABLE void deleteHistory(QString id);
    Q_INVOKABLE QString getLocale();
    Q_INVOKABLE QVariant getTs(QString);
    Q_INVOKABLE void downloadFiles(QString, QString);

signals:
    void memoChanged(const QString &memo, QString);
    void incomingMessage(QString,QString);
    void textChanged(QString);
Q_SIGNALS:
    void attachmentProgres(qint64,qint64);
    void attachmentFinish(QString);
private slots:
    void onInvoked(const bb::system::InvokeRequest& request);
    void cardPooled(const bb::system::CardDoneMessage& doneMessage);
    void settingsChanged(const QString&);
    void onRequestCall();
    void readReady();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
            void error(QNetworkReply::NetworkError code);
            void finished();

private:
    static const QString m_author; // for creating settings
    static const QString m_appName; // for creating settings
    QFileSystemWatcher* settingsWatcher;
    Database *db;
    Uploader *uploader;
    DownloadManager *dm;

};

#endif /* CardUI_HPP_ */
