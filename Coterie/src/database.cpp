/*
 * database.cpp
 *
 *  Created on: 17-Dec-2014
 *      Author: perl
 */
#include "database.hpp"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtSql/QtSql>
#include <QDate>
#include <QDebug>
#include <QObject>
#include <bb/cascades/GroupDataModel>
#include <bb/data/SqlDataAccess>
using namespace bb::cascades;
using namespace bb::data;

//class bb::cascades::GroupDataModel;
Database::Database(QObject *parent) :
        QObject(parent), DB_PATH("./data/Slack.db")
{
    sqlda = new SqlDataAccess(DB_PATH);
    qDebug() << "thisi is the DB" << DB_PATH;
    initDatabase();
}
Database::~Database()
{
}
bool Database::openDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(DB_PATH);
    qDebug() << db.isValid();
    qDebug() << db.open();
    bool ok = db.open();
    if (ok) {
        SLACK_TABLES = db.tables(QSql::Tables);
    }
    return ok;
}
bool Database::initDatabase()
{ //call this method with the name of the database with
    if (openDatabase()) {
        qDebug() << "Slack database  created";
        QString muteChannelsTable = "create table if not exists mutechannels(channelid primary key unique)";
              QSqlQuery queryMuteChannels(muteChannelsTable);

              qDebug() << " mute channels  table created" << queryMuteChannels.isActive();
        QString settingsTable =
                "create table if not exists settings(type varchar,value varchar primary key unique,account varchar,active varchar , id varchar)";
        QSqlQuery queryAuths(settingsTable);

        qDebug() << " settings table created" << queryAuths.isActive();

        QString msgsTable = "create table if not exists msgs(channel,user,text,ts primary key unique,type varchar , id varchar)";
               QSqlQuery queryMsgs(msgsTable);

               qDebug() << " msgs  table created" << queryMsgs.isActive();

        QString appsettingsTable =
                        "create table if not exists appsettings(type primary key unique,value)";
                QSqlQuery appqueryAuths(appsettingsTable);

                qDebug() << " appsettings table created" << appqueryAuths.isActive();

        QString notificationsTable =
                "create table if not exists notifications(channel varchar primary key unique)";
        QSqlQuery queryNotifications(notificationsTable);

        qDebug() << " notifications  table created" << queryNotifications.isActive();

        QString downloadsTable =
                "create table if not exists dowloads(id primary key unique,channel,location,size,type)";
        QSqlQuery queryDownloads(downloadsTable);

        qDebug() << " downloads table created" << queryDownloads.isActive();
        QString chanelsTable = "create table if not exists channels(id primary key unique,name,type)";
        QSqlQuery queryChannels(chanelsTable);

        qDebug() << " channles table created" << queryChannels.isActive();

        QString loggerTable = "create table if not exists logger(timestamp,text)";
        QSqlQuery queryLogger(loggerTable);

        qDebug() << " logger table created" << queryLogger.isActive();


        return true;
    } else
        return false;
}

QVariant Database::executeQuery(QString q)
{
    return sqlda->execute(q);
}
QSqlQuery Database::executeSqlQuery(QString q)
{
    QSqlQuery query(q);
    return query;
}

GroupDataModel * Database::getQueryModel(QString query)
{
    GroupDataModel *model = new GroupDataModel(QStringList());
    QVariant data = sqlda->execute(query);
    model->insertList(data.value<QVariantList>());
    return model;
}
int Database::getTableSizeByQuery(QString query)
{
    QSqlQuery q;
    q.prepare(query);
    if (q.exec()) {
        int rows = 0;
        if (q.next()) {
            rows = q.value(0).toInt();
        }
        return rows;
    } else
        qDebug() << q.lastError();
}
int Database::getTableSize(QString tabname)
{
    QSqlQuery q;
    q.prepare(QString("SELECT COUNT (*) FROM %1").arg(tabname));
    if (q.exec()) {
        int rows = 0;
        if (q.next()) {
            rows = q.value(0).toInt();
        }
        return rows;
    } else
        qDebug() << q.lastError();
}

bool Database::insertQuery(QString query, QVariantMap bind)
{
    sqlda->execute(query, bind);
    return sqlda->hasError();
}

void Database::deleteTable(QString tablename)
{
    QSqlQuery q(QString("delete from  %1").arg(tablename));
}
