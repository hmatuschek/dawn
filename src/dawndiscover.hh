#ifndef DAWNDISCOVER_HH
#define DAWNDISCOVER_HH

#include <QObject>
#include <QBluetoothLocalDevice>
#include <QBluetoothServiceDiscoveryAgent>

class DawnDiscover : public QObject
{
  Q_OBJECT
public:
  explicit DawnDiscover(QObject *parent = 0);

public slots:
  bool start();

protected slots:
  void deviceDiscovered(const QBluetoothDeviceInfo &device);
  void finished();

protected:
  QBluetoothLocalDevice _local;
  QBluetoothDeviceDiscoveryAgent _discovery;
};

#endif // DAWNDISCOVER_HH
