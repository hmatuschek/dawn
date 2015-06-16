#include <QApplication>
#include "portdialog.hh"
#include <QSerialPort>
#include <QMessageBox>
#include <QDateTime>

#include "dawn.hh"
#include "mainwindow.hh"

#include <iostream>
#include <time.h>

#include "secret.h"

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
    dawn = new Dawn(systemLocation, secret);

    /*if (! dawn->isValid()) {
      QMessageBox::critical(
            0, QObject::tr("Got invalid time from device"),
            QObject::tr("Got invalid time from device at interface %1 (%2)"
                        ).arg(name).arg(systemLocation));
      delete dawn;
      continue;
    }*/
    break;
  }

  // Create main window
  MainWindow window(*dawn);
  window.show();

  // go...
  app.exec();

  return 0;
}
