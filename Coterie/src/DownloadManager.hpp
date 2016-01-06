#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QObject>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkReply>
#include <QFile>
#include <QFileInfo>
//#include <QDesktopServices>
#include <QQueue>
#include <QFileInfo>
#include <QStringList>
#include <QVariant>
#include <QPair>
#include <QUrl>

#define CACHE_SIZE 128 * 1024 * 1024
#define MAX_DOWNLOADS 16

class DownloadManager: public QObject
{
    Q_OBJECT
public:
    DownloadManager(QObject *parent = 0);
    QString downloadDirectory;
    QNetworkReply *reply;
    QQueue<QPair<QUrl, QString> > queue;
    QHash<QString, QFile *> downloads;
    QNetworkAccessManager manager;

    void nextDownload();

signals:
    void requestCall();
    void donwloadscomplete();
public slots:
    void append(const QUrl& url, const QString& filename);
    void remove(const QString& filename);

protected slots:


private:
    Q_DISABLE_COPY (DownloadManager)
    QNetworkDiskCache cache;

};

#endif // DOWNLOADMANAGER_H
