#include "devicesettings.hh"
#include <QSettings>


/* ********************************************************************************************* *
 * Implementation of DeviceSettings::Device
 * ********************************************************************************************* */
DeviceSettings::Device::Device()
  : _name(), _device(), _secret()
{
  // pass...
}

DeviceSettings::Device::Device(const QString &name, const QString &device, const QByteArray secret)
  : _name(name), _device(device), _secret(secret)
{
  // pass...
}

DeviceSettings::Device::Device(const Device &other)
  : _name(other._name), _device(other._device), _secret(other._secret)
{
  // pass...
}

DeviceSettings::Device &
DeviceSettings::Device::operator =(const DeviceSettings::Device &other) {
  _name = other._name;
  _device = other._device;
  _secret = other._secret;
  return *this;
}

const QString &
DeviceSettings::Device::name() const {
  return _name;
}

const QString &
DeviceSettings::Device::device() const {
  return _device;
}

const QByteArray &
DeviceSettings::Device::secret() const {
  return _secret;
}


/* ********************************************************************************************* *
 * Implementation of DeviceSettings
 * ********************************************************************************************* */
DeviceSettings::DeviceSettings()
{
  QSettings settings("hmatuschek.github.io", "dawn");
  int num_devices = settings.beginReadArray("devices");
  for (int i=0; i<num_devices; i++) {
    settings.setArrayIndex(i);
    Device dev(settings.value("name").toString(), settings.value("port").toString(),
               QByteArray::fromHex(settings.value("secret").toByteArray()));
    _devices[dev.name()] = dev;
  }
  settings.endArray();
}

bool
DeviceSettings::hasDevices() const {
  return 0 != _devices.size();
}

bool
DeviceSettings::hasDevice(const QString &name) const {
  return _devices.contains(name);
}

QStringList
DeviceSettings::devices() const {
  return _devices.keys();
}

DeviceSettings::Device
DeviceSettings::device(const QString &name) const {
  return _devices[name];
}

void
DeviceSettings::add(const QString &name, const QString &device, const QByteArray &secret) {
  _devices.insert(name, Device(name, device, secret));

  QSettings settings("hmatuschek.github.io", "dawn");
  settings.beginWriteArray("devices");
  QHash<QString, Device>::iterator item = _devices.begin();
  for (int i=0; item != _devices.end(); item++, i++) {
    settings.setArrayIndex(i);
    settings.setValue("name", item->name());
    settings.setValue("port", item->device());
    settings.setValue("secret", item->secret().toHex());
  }
  settings.endArray();
}
