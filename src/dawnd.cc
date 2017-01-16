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
    doc = onListAlarm(query, request);
  else if (query.hasQueryItem("q") && ("temp" == query.queryItemValue("q")))
    doc = onGetTemp(query, request);

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
Application::onListAlarm(const QUrlQuery &query, QFCgiRequest *request) {
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
Application::onGetTemp(const QUrlQuery &query, QFCgiRequest *request) {
  double core, amb;
  bool success = false;
  for (int i=0; (i<5) && (! success); i++)
    success = _dawn.getTemp(core, amb);

  if (success) {
    QJsonObject res; res.insert("temp", amb);
    return QJsonDocument(res);
  }

  return QJsonDocument();
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
