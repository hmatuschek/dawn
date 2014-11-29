#include <QApplication>
#include "portdialog.hh"
#include <QSerialPort>
#include <QMessageBox>
#include <QDateTime>

#include "dawn.hh"
#include "mainwindow.hh"

#include <iostream>
#include <time.h>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QString name, systemLocation;

  // Let the user select an interfact to the device:
  while (true) {
    PortDialog dialog;
    if (QDialog::Accepted != dialog.exec()) { return 0; }

    name = dialog.name();
    systemLocation = dialog.systemLocation();

    QSerialPort port;
    port.setPortName(systemLocation);
    port.setBaudRate(9600);
    if (! port.open(QIODevice::ReadWrite)) {
      QMessageBox::critical(
            0, QObject::tr("Can not open interface"),
            QObject::tr("Can not open interface %1 (%2)"
                        ).arg(name).arg(systemLocation));
      continue;
    }

    // Try to get time from device
    port.write("TIME\n");
    port.waitForBytesWritten(1000);

    QString line;
    while(! line.contains('\n')) {
      if (! port.waitForReadyRead(1000)) { break; }
      line.append(port.readAll());
    }

    QDateTime time = QDateTime::fromString(line.simplified(), "yyyy-M-d H:m:s");
    if (! time.isValid()) {
      QMessageBox::critical(
            0, QObject::tr("Got invalid time from device"),
            QObject::tr("Got invalid time from device at interface %1 (%2)"
                        ).arg(name).arg(systemLocation));
      continue;
    }
    break;
  }

  // Create interface object
  Dawn dawn(systemLocation);

  // Create main window
  MainWindow window(dawn);
  window.show();

  // go...
  app.exec();

  return 0;
}
