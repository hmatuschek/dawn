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

    dawn = new Dawn(systemLocation, (const uint8_t *)secret.constData());

    if (dawn->isValid()) {
      break;
    } else {
      QMessageBox::critical(
            0, QObject::tr("Can not access device."),
            QObject::tr("Can not access device at interface %1 (%2).").arg(
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
