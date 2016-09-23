#include <QApplication>
#include <QSerialPort>
#include <QMessageBox>
#include <QDateTime>
#include <QTranslator>
#include <QSettings>

#include "portdialog.hh"
#include "dawn.hh"
#include "mainwindow.hh"
#include "logger.hh"

#include <iostream>
#include <time.h>


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  app.setApplicationName("dawn");
  app.setOrganizationDomain("hmatuschek.github.io");

  QTranslator translator;
  translator.load("://i18n/dawn.qm");
  app.installTranslator(&translator);

  QSettings settings("hmatuschek.github.io", "dawn");

  Dawn *dawn = 0;
  QString name, systemLocation;
  QByteArray secret;
  // Let the user select an interfact to the device:
  while (true) {
    DeviceDialog dialog(settings);
    if (QDialog::Accepted != dialog.exec()) { return 0; }

    name = dialog.name();
    systemLocation = dialog.systemLocation();
    secret = dialog.secret();
    if (16 != secret.size()) { secret.resize(16); }

    QSerialPort *port = new QSerialPort(systemLocation);

    // Open port
    port->open(QIODevice::ReadWrite);
    if (! port->isOpen()) {
      LogMessage msg(LOG_ERROR);
      msg << "IO Error: Can not open device " << systemLocation.toStdString();
      Logger::get().log(msg);
      goto error;
    }
    if (! port->setBaudRate(QSerialPort::Baud38400)) {
      LogMessage msg(LOG_ERROR);
      msg << "IO: Can not set baudrate.";
      Logger::get().log(msg);
      goto error;
    }
    if (! port->setDataBits(QSerialPort::Data8)) {
      LogMessage msg(LOG_ERROR);
      msg << "IO: Can not set data bits.";
      Logger::get().log(msg);
      goto error;
    }
    if (! port->setParity(QSerialPort::NoParity)) {
      LogMessage msg(LOG_ERROR);
      msg << "IO: Can not set parity.";
      Logger::get().log(msg);
      goto error;
    }
    if (! port->setStopBits(QSerialPort::OneStop)) {
      LogMessage msg(LOG_ERROR);
      msg << "IO: Can not set stop bits.";
      Logger::get().log(msg);
      goto error;
    }
    if (! port->setFlowControl(QSerialPort::HardwareControl)) {
      LogMessage msg(LOG_ERROR);
      msg << "IO: Can not set stop bits.";
      Logger::get().log(msg);
      goto error;
    }

    dawn = new Dawn(port, (const uint8_t *)secret.constData());

    break;

error:
      QMessageBox::critical(
            0, QObject::tr("Can not access device."),
            QObject::tr("Can not access device at interface %1 (%2).").arg(
              name, systemLocation));
      if (dawn)
        delete dawn;
      else
        delete port;
      dawn = 0;
  }

  // Create main window
  MainWindow window(*dawn);
  window.show();

  // go...
  app.exec();

  return 0;
}
