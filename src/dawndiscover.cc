#include "dawndiscover.hh"
#include <iostream>
#include <QEventLoop>
#include <QTimer>
#include <QCoreApplication>

DawnDiscover::DawnDiscover(const QBluetoothAddress &device, QObject *parent)
  : QObject(parent), _local(device), _discovery(device)
{
  if (! _local.isValid()) {
    std::cerr << "No valid local BT device." << std::endl;
    return;
  }

  _local.powerOn();
  _local.setHostMode(QBluetoothLocalDevice::HostDiscoverable);

  connect(&_discovery, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
          this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));
  connect(&_discovery, SIGNAL(finished()), this, SLOT(finished()));
  connect(&_discovery, SIGNAL(error(QBluetoothDeviceDiscoveryAgent::Error)),
          this, SLOT(finished()));
}

bool
DawnDiscover::start() {
  if (! _local.isValid()) {
    std::cerr << "No valid local BT device." << std::endl;
    return false;
  }
  if (QBluetoothDeviceDiscoveryAgent::NoError != _discovery.error()) {
    std::cerr << "Cannot scan: " << _discovery.errorString().toStdString() << std::endl;
    return false;
  }
  std::cerr << "Scan for devices..." << std::endl;
  _discovery.start();
  return true;
}


void
DawnDiscover::deviceDiscovered(const QBluetoothDeviceInfo &device) {
  std::cout << "Found " << device.address().toString().toStdString()
            << ": " << device.name().toStdString() << std::endl;
}

void
DawnDiscover::finished() {
  std::cerr << "done." << std::endl;
  QCoreApplication::instance()->quit();
}
