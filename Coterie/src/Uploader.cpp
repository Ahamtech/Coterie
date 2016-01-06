#include "Uploader.hpp"
#include <QObject>
#include <QDebug>
#include <QtNetwork/QHttpPart>
#include <QtNetwork/QHttpMultiPart>
#include <QPair>
#include <QUrl>
#include <QStringList>
#include <QBuffer>
#include <QFile>

#include "freedesktopmime.h"
Uploader::Uploader(QObject *parent) :
        QObject(parent), manager(new QNetworkAccessManager(this))
{

}

Uploader::~Uploader()
{
//
//    if (manager)
//        delete manager;
    // TODO Auto-generated destructor stub
}

void Uploader::uploadFile(QString filePath, QString token, QString channel, QString title)
{
    QPair<QString, QVariantMap> pair;
    QVariantMap map;
    map["channel"] = channel;
    map["token"] = token;
    map["title"] = title;
    pair.first = filePath;

    QStringList list = filePath.split("/");
    QString fileName = list.last();
    qDebug() << "this is the file name we are uploading " << fileName;
    map["fileName"] = fileName;
    pair.second = map;
    queue.enqueue(pair);
    nextDownLoad();
}

void Uploader::slotFinished()
{
    m_reply->deleteLater();
}
void Uploader::slotUploadProgress(qint64 rem, qint64 total)
{
    qDebug() << "load Progress is called ";
    qDebug() << m_reply->url().toString() << rem << total;
}

void Uploader::nextDownLoad()
{
    if (!queue.isEmpty()) {
        QPair<QString, QVariantMap> pair = queue.dequeue();
        QVariantMap map = pair.second;
        QString title = map["title"].toString();
        QString fileName = map["fileName"].toString();
        QString filePath = pair.first;
        QString token = map["token"].toString();
        QString channel = map["channel"].toString();



        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        qDebug() << multiPart->boundary() << "this is the boundary added";

        QHttpPart channelPart;
        channelPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                QVariant("form-data; name=\"channels\""));
        channelPart.setBody(channel.toUtf8());

        QHttpPart titlePart;
        titlePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                QVariant("form-data; name=\"title\""));
        titlePart.setBody(title.toUtf8());

        QHttpPart tokenPart;
        tokenPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                QVariant("form-data; name=\"token\""));
        tokenPart.setBody(token.toUtf8());

        QHttpPart fileNamePart;
        fileNamePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                QVariant("form-data; name=\"filename\""));
        fileNamePart.setBody(fileName.toUtf8());

        QFile *file = new QFile(filePath);

        QHttpPart filePart;
        QNetworkRequest request(QUrl(QString("https://slack.com/api/files.upload")));
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("multipart/form-data"));
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name= \"content\""));
        file->open(QIODevice::ReadOnly);
        filePart.setBodyDevice(file);

        multiPart->append(filePart);
        multiPart->append(channelPart);
        multiPart->append(titlePart);
        multiPart->append(fileNamePart);
        multiPart->append(tokenPart);

        QObject::connect(manager, SIGNAL(finished(QNetworkReply*)), this,SIGNAL(signalFinished(QNetworkReply*)));
        m_reply = manager->post(request, multiPart);
        QObject::connect(m_reply, SIGNAL(readyRead()), this, SIGNAL(signalReadyRead()));
        QObject::connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), this,SIGNAL(signalError(QNetworkReply::NetworkError)));
        QObject::connect(m_reply, SIGNAL(uploadProgress(qint64, qint64)), this,SLOT(slotUploadProgress(qint64, qint64)));
        QObject::connect(m_reply, SIGNAL(finished()), this, SLOT(slotFinished()));

    }
}
