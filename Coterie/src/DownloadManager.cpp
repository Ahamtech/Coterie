/*
 * DownloadManagers.cpp
 *
 *  Created on: 21-Feb-2015
 *      Author: ROG
 */
#include "DownloadManager.hpp"
#include <QDir>
#include <QDebug>
DownloadManager::DownloadManager(QObject *parent) :
        QObject(parent), downloadDirectory("./data/ahammedia")
{
    cache.setMaximumCacheSize(CACHE_SIZE);
    cache.setCacheDirectory(downloadDirectory);
    manager.setCache(&cache);
}

void DownloadManager::append(const QUrl& url, const QString& filename)
{
    queue.enqueue(qMakePair(url, filename));
    nextDownload();
}

void DownloadManager::remove(const QString& filename)
{
    QFile *file = downloads[filename];
    file->close();
    file->deleteLater();
    downloads.remove(filename);
}

void DownloadManager::nextDownload()
{
    if (downloads.size() < MAX_DOWNLOADS && !queue.empty()) {
        QPair<QUrl, QString> download = queue.dequeue();
        QUrl url = download.first;
        QString filename = "./data/ahammedia/" + download.second;
        QFile *file = new QFile(filename, this);
        if (!file->open(QIODevice::ReadWrite)) {
            delete file;
            return;
        }
        QNetworkRequest request(url);
        request.setAttribute(QNetworkRequest::User, filename);
        request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                QNetworkRequest::PreferCache);
        reply = manager.get(request);
        emit requestCall();
        downloads.insert(filename, file);
    }else
        emit donwloadscomplete();

}

