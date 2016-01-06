/*
 * Stream.cpp
 *
 *  Created on: 06-Apr-2015
 *      Author: Winasus
 */

#include "Stream.h"
#include <QObject>
#include <QDebug>
#include "service.hpp"
class Service;
Stream::Stream(QObject *parent) :
        QObject(parent), manager(new QNetworkAccessManager(this))
{

connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(parsingRTMResp(QNetworkReply*)));
}

Stream::~Stream()
{
//if (manager)
//    delete manager;
// TODO Auto-generated destructor stub

}

void Stream::stream_url(QString url)
{
request.setUrl(QUrl(url));
request.setRawHeader("User-Agent", "BB 10");
request.setRawHeader("content-type", "application/json");
m_reply = manager->get(request);

}
void Stream::parsingRTMResp(QNetworkReply* reply)
{
  if(!reply->error())
    Q_EMIT parseRTM(reply->readAll());
    qDebug() << "Stream)url signal";
    reply->deleteLater();
}
