#ifndef DAWND_HH
#define DAWND_HH

#include "dawn.hh"
#include "qfcgi.h"
#include <QUrlQuery>
#include <QCoreApplication>
#include <QJsonDocument>


class Application: public QCoreApplication
{
  Q_OBJECT

public:
  Application(Dawn &dawn, int &argc, char *argv[]);

private slots:
  void onNewRequest(QFCgiRequest *request);
  QJsonDocument onListAlarm(const QUrlQuery &query);
  QJsonDocument onSetAlarm(const QUrlQuery &query);
  QJsonDocument onGetTemp(const QUrlQuery &query);
  QJsonDocument onGetTime(const QUrlQuery &query);
  QJsonDocument onSetTime(const QUrlQuery &query);
  QJsonDocument onGetValue(const QUrlQuery &query);
  QJsonDocument onSetValue(const QUrlQuery &query);

protected:
  Dawn &_dawn;
  QFCgi *_fcgi;
};

#endif // DAWND_HH
