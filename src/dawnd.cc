#include "dawnd.hh"
#include "dawn.hh"
#include <iostream>
#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <QHostAddress>
#include "option_parser.hh"
#include "devicesettings.hh"
#include "dawndiscover.hh"
#include <QLoggingCategory>
#include <signal.h>
#include <QUrl>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>


Application::Application(Dawn &dawn, int &argc, char *argv[])
  : QCoreApplication(argc, argv), _dawn(dawn), _fcgi(0)
{
  _fcgi = new QFCgi(this);
  connect(_fcgi, SIGNAL(newRequest(QFCgiRequest*)),
          this, SLOT(onNewRequest(QFCgiRequest*)));

  // confiure the FastCGI application server to listen on stdin
  _fcgi->configureListen(QHostAddress::LocalHost, 9001);
  // start the FastCGI application server
  _fcgi->start();
  // check the status of the FastCGI application server
  if (! _fcgi->isStarted())
    quit();
}

void
Application::onNewRequest(QFCgiRequest *request) {
  QUrl requestURI = QUrl::fromEncoded(request->getParam("REQUEST_URI").toUtf8());
  qDebug() << requestURI;
  QUrlQuery query(requestURI);
  qDebug() << query.toString();
  QJsonDocument doc;

  if (query.hasQueryItem("q") && ("list" == query.queryItemValue("q")))
    doc = onListAlarm(query);
  if (query.hasQueryItem("q") && ("setalarm" == query.queryItemValue("q")))
    doc = onSetAlarm(query);
  else if (query.hasQueryItem("q") && ("temp" == query.queryItemValue("q")))
    doc = onGetTemp(query);
  else if (query.hasQueryItem("q") && ("time" == query.queryItemValue("q")))
    doc = onGetTime(query);
  else if (query.hasQueryItem("q") && ("settime" == query.queryItemValue("q")))
    doc = onSetTime(query);
  else if (query.hasQueryItem("q") && ("value" == query.queryItemValue("q")))
    doc = onGetValue(query);
  else if (query.hasQueryItem("q") && ("setvalue" == query.queryItemValue("q")))
    doc = onSetValue(query);

  QByteArray buffer = doc.toJson();
  QTextStream ts(request->getOut());
  ts << "Content-Type: application/json\r\n"
     << "Content-Length: " << buffer.size() << "\r\n"
     << "\r\n"
     << buffer;
  ts.flush();

  request->endRequest(0);
}

QJsonDocument
Application::onListAlarm(const QUrlQuery &query) {
  QJsonArray lst;
  for (size_t i=0; i<_dawn.numAlarms(); i++) {
    QJsonObject alarm;
    alarm.insert("dayofweek", int(_dawn.alarm(i).dowFlags));
    alarm.insert("time", _dawn.alarm(i).time.toString("hh:mm"));
    lst.append(alarm);
  }

  return QJsonDocument(lst);
}

QJsonDocument
Application::onSetAlarm(const QUrlQuery &query) {
  if ((! query.hasQueryItem("idx")) || (! query.hasQueryItem("dow")) || (! query.hasQueryItem("time")))
    return QJsonDocument();

  bool ok; Dawn::Alarm alarm;
  int idx = query.queryItemValue("idx").toInt(&ok);
  if (!ok) return QJsonDocument();

  alarm.dowFlags = (query.queryItemValue("dow").toInt(&ok) & 0b1111111);
  if (!ok) return QJsonDocument();

  alarm.time = QTime::fromString(query.queryItemValue("time"), "hh:mm");
  if (! alarm.time.isValid()) return QJsonDocument();

  ok = false;
  for (int i=0; (i<5) && (!ok); i++)
    ok = _dawn.setAlarm(idx, alarm);
  if (! ok) return QJsonDocument();

  return QJsonDocument(QJsonObject());
}

QJsonDocument
Application::onGetTemp(const QUrlQuery &query) {
  double core, amb;
  bool success = false;
  for (int i=0; (i<5) && (! success); i++)
    success = _dawn.getTemp(core, amb);

  if (! success)
    return QJsonDocument();

  QJsonObject res; res.insert("temp", amb);
  return QJsonDocument(res);
}

QJsonDocument
Application::onGetTime(const QUrlQuery &query) {
  QDateTime time;
  bool ok = false;
  for (int i=0; (i<5) && (! ok); i++)
    time = _dawn.time(&ok);
  if (! ok) return QJsonDocument();

  QJsonObject res;
  res.insert("time", time.toString("yyyy-MM-dd hh:mm"));
  return QJsonDocument(res);
}

QJsonDocument
Application::onSetTime(const QUrlQuery &query) {
  QDateTime time = QDateTime::currentDateTime();
  if (query.hasQueryItem("time")) {
    QDateTime tmp = QDateTime::fromString(query.queryItemValue("time"), "yyyy-MM-dd hh:mm");
    if (tmp.isValid())
      time = tmp;
  }

  bool ok = false;
  for (int i=0; (i<5) && (! ok); i++)
    ok = _dawn.setTime(time);
  if (! ok) return QJsonDocument();

  return QJsonDocument(QJsonObject());
}

QJsonDocument
Application::onGetValue(const QUrlQuery &query) {
  uint16_t value; bool ok = false;
  for (int i=0; (i<5) && (! ok); i++)
    value = _dawn.value(&ok);
  if (! ok) return QJsonDocument();

  QJsonObject res; res.insert("value", value);
  return QJsonDocument(res);
}

QJsonDocument
Application::onSetValue(const QUrlQuery &query) {
  if (! query.hasQueryItem("value"))
    return QJsonDocument();
  uint16_t value = query.queryItemValue("value").toUInt();
  bool ok = false;
  for (int i=0; (i<5) && (! ok); i++)
    ok = _dawn.setValue(value);
  if (! ok) return QJsonDocument();
  return QJsonDocument(QJsonObject());
}



static void quit_handler(int signal) {
  qApp->quit();
}

int main(int argc, char *argv[]) {
  Logger::get().addHandler(new StreamLogHandler(LOG_DEBUG, std::cerr));

  Parser parser;
  parser.setGrammar( (parser.Option("device"), parser.Option("secret")) );

  if (! parser.parse((const char **)argv, argc)) {
    std::cout << "Usage " << parser.format_help("dawnd") << std::endl;
    return -1;
  }

  struct sigaction sa;

  memset(&sa, 0, sizeof(struct sigaction));
  sa.sa_handler = quit_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGHUP, &sa, 0) != 0 ||
      sigaction(SIGTERM, &sa, 0) != 0 ||
      sigaction(SIGINT, &sa, 0) != 0) {
    perror(argv[0]);
    return 1;
  }

  QString portname = QString::fromStdString(parser.get_option("device").front());
  QByteArray secret = QByteArray::fromHex(parser.get_option("secret").front().c_str());

  QSerialPort *port = new QSerialPort(portname);
  if (0 == port) {
    LogMessage msg(LOG_ERROR);
    msg << "IO Error: Can not open device " << portname.toStdString();
    Logger::get().log(msg);
    return -1;
  }

  // Open port
  port->open(QIODevice::ReadWrite);
  if (! port->isOpen()) {
    LogMessage msg(LOG_ERROR);
    msg << "IO Error: Can not open device " << portname.toStdString();
    Logger::get().log(msg);
    delete port;
    return -1;
  }
  if (! port->setBaudRate(QSerialPort::Baud9600)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set baudrate.";
    Logger::get().log(msg);
    delete port;
    return -1;
  }
  if (! port->setDataBits(QSerialPort::Data8)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set data bits.";
    Logger::get().log(msg);
    delete port;
    return -1;
  }
  if (! port->setParity(QSerialPort::NoParity)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set parity.";
    Logger::get().log(msg);
    delete port;
    return -1;
  }
  if (! port->setStopBits(QSerialPort::OneStop)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set stop bits.";
    Logger::get().log(msg);
    delete port;
    return -1;
  }
  if (! port->setFlowControl(QSerialPort::HardwareControl)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set stop bits.";
    Logger::get().log(msg);
    delete port;
    return -1;
  }

  Dawn dawn(port, (const uint8_t *)secret.constData(), true);
  if (! dawn.isValid()) {
    std::cerr << "Failed to access device." << std::endl;
    return -1;
  }

  Application app(dawn, argc, argv);
  app.setOrganizationDomain("hmatuschek.github.io");
  app.setApplicationName("dawnd");

  app.exec();
}
