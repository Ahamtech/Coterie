/*
 * Stream.cpp
 *
 *  Created on: 06-Apr-2015
 *      Author: Winasus
 */

#include "Stream.h"
#include <QObject>
#include <QDebug>

Stream::Stream(QObject *parent):
QObject(parent),manager(new QNetworkAccessManager(this))
{
}

Stream::~Stream()
{
    if(manager) delete manager;
    // TODO Auto-generated destructor stub
}

void Stream::stream_url(QString url){

    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", "MyOwnBrowser 1.0");

    m_reply = manager->get(request);
   connect(m_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
   connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

//    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
//            this, SLOT(slotError(QNetworkReply::NetworkError)));
//    connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
//            this, SLOT(slotSslErrors(QList<QSslError>)));
}
void Stream::slotReadyRead(){
    QByteArray response = m_reply->readAll();

    qDebug()<< "slor ready "<< response;
}
 void Stream::replyFinished(QNetworkReply* reply){

     qDebug()<<"this is the total data"<<reply->readAll();
 }
