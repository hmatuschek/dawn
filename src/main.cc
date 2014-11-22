#include <QApplication>
#include "portdialog.hh"
#include <QSerialPort>
#include <QMessageBox>
#include <QDateTime>
#include "dawn.hh"


int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  QString name, systemLocation;

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
            QObject::tr("Can not open interface %1 (%2)").arg(name).arg(systemLocation));
      continue;
    }

    // Try to get time from device
    port.write("TIME\n");
    if (! port.waitForReadyRead(1000)) {
      QMessageBox::critical(
            0, QObject::tr("Can not read time"),
            QObject::tr("Can not read time from interface %1 (%2)").arg(name).arg(systemLocation));
      continue;
    }

    QString line = port.readLine();
    QDateTime time = QDateTime::fromString(line, "yyyy-M-d h:m:s");
    if (! time.isValid()) {
      QMessageBox::critical(
            0, QObject::tr("Got invalid time from device"),
            QObject::tr("Got invalid time from device at interface %1 (%2)").arg(name).arg(systemLocation));
      continue;
    }
    break;
  }

  // Create interface object
  Dawn dawn(systemLocation);

  return 0;
}
