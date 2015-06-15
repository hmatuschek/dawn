#include <QApplication>
#include "portdialog.hh"
#include <QSerialPort>
#include <QMessageBox>
#include <QDateTime>

#include "dawn.hh"
#include "mainwindow.hh"

#include <iostream>
#include <time.h>


unsigned const char device_secret[] =
 { 0xf3, 0xc5, 0x97, 0x79, 0xa4, 0x27, 0xa6, 0xaf,
   0xc6, 0xae, 0x14, 0x81, 0xdd, 0xb8, 0x5c, 0xa0 };


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  Dawn *dawn = 0;
  QString name, systemLocation;
  // Let the user select an interfact to the device:
  while (true) {
    PortDialog dialog;
    if (QDialog::Accepted != dialog.exec()) { return 0; }

    name = dialog.name();
    systemLocation = dialog.systemLocation();
    dawn = new Dawn(systemLocation, device_secret);

    if (! dawn->isValid()) {
      QMessageBox::critical(
            0, QObject::tr("Got invalid time from device"),
            QObject::tr("Got invalid time from device at interface %1 (%2)"
                        ).arg(name).arg(systemLocation));
      delete dawn;
      continue;
    }
    break;
  }

  // Create main window
  MainWindow window(*dawn);
  window.show();

  // go...
  app.exec();

  return 0;
}
