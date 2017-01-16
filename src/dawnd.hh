#ifndef DAWND_HH
#define DAWND_HH

#include "dawn.hh"
#include "qfcgi.h"
#include <QCoreApplication>


class Application: public QCoreApplication
{
  Q_OBJECT

public:
  Application(Dawn &dawn, int &argc, char *argv[]);

private slots:
  void onNewRequest(QFCgiRequest *request);

protected:
  Dawn &_dawn;
  QFCgi *_fcgi;
};

#endif // DAWND_HH
