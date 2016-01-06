/*
 * Stream.h
 *
 *  Created on: 06-Apr-2015
 *      Author: Winasus
 */

#ifndef STREAM_H_
#define STREAM_H_
#include<QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>
#include <QObject>
class QNetworkReply;
class Stream:public QObject
{
    Q_OBJECT
public:
    Stream(QObject*);
    virtual ~Stream();
    void stream_url(QString);
private:
    QNetworkReply *m_reply;
    QNetworkRequest request;
    QNetworkAccessManager* manager;
private slots:
void slotReadyRead();
void replyFinished(QNetworkReply* reply);
};

#endif /* STREAM_H_ */
