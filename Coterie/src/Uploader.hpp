/*
 * Uploader.hpp
 *
 *  Created on: 15-Apr-2015
 *      Author: Winasus
 */

#ifndef UPLOADER_HPP_
#define UPLOADER_HPP_

#include<QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QObject>
#include <QPair>
#include <QQueue>
class QNetworkReply;
typedef QPair<QString,QVariantMap> mytype;
class Uploader:public QObject
{
    Q_OBJECT
public:
    Uploader(QObject*);
    virtual ~Uploader();
    void uploadFile(QString,QString,QString,QString);
private:
    QNetworkReply *m_reply;
    QNetworkRequest request;
    QNetworkAccessManager* manager;
    QQueue<mytype> queue;
    void nextDownLoad();

signals:
void signalReadyRead();
void signalError(QNetworkReply::NetworkError);
void signalFinished(QNetworkReply*);
void signalUploadProgress (qint64, qint64);
void upProgress(QString,qint64, qint64);
private slots:
void slotFinished();
void slotUploadProgress (qint64, qint64);
};

#endif

