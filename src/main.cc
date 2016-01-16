#include <QApplication>
#include <QSerialPort>
#include <QMessageBox>
#include <QDateTime>
#include <QTranslator>

#include "portdialog.hh"
#include "dawn.hh"
#include "mainwindow.hh"
#include "logger.hh"

#include <iostream>
#include <time.h>

#include "secret.h"

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QTranslator translator;
  translator.load("://i18n/dawn.qm");
  app.installTranslator(&translator);

  // Register log-handler
  Logger::get().addHandler(
        new StreamLogHandler(LOG_DEBUG, std::cerr));

  Dawn *dawn = 0;
  QString name, systemLocation;
  // Let the user select an interfact to the device:
  while (true) {
    PortDialog dialog;
    if (QDialog::Accepted != dialog.exec()) { return 0; }

    name = dialog.name();
    systemLocation = dialog.systemLocation();
    dawn = new Dawn(systemLocation, secret);

    if (dawn->isValid()) {
      break;
    } else {
      QMessageBox::critical(
            0, QObject::tr("Can not access device."),
            QObject::tr("Can not access device at interface %1 (%2)").arg(
              name, systemLocation));
      delete dawn;
    }
  }

  // Create main window
  MainWindow window(*dawn);
  window.show();

  // go...
  app.exec();

  return 0;
}
