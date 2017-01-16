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
    QTextStream ts(request->getOut());

    ts << "Content-Type: text/plain\r\n";
    ts << "\r\n";
    ts << QString("Hello from %1\n").arg(this->applicationName());
    ts << "This is what I received:\n";
    foreach (QString key, request->getParams()) {
      ts << QString("%1: %2\n").arg(key).arg(request->getParam(key));
    }
    ts.flush();

    request->endRequest(0);
  }


int main(int argc, char *argv[]) {
  Logger::get().addHandler(new StreamLogHandler(LOG_DEBUG, std::cerr));

  Parser parser;
  parser.setGrammar( (parser.Option("device"), parser.Option("secret")) );

  if (! parser.parse((const char **)argv, argc)) {
    std::cout << "Usage " << parser.format_help("dawnd") << std::endl;
    return -1;
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
