#include "dawn.hh"
#include <iostream>
#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include "option_parser.hh"

class DeviceSettings
{
public:
  class Device {
  public:
    Device();
    Device(const QString &name, const QString &device, const QByteArray secret);
    Device(const Device &other);

    Device &operator= (const Device &other);

    const QString &name() const;
    const QString &device() const;
    const QByteArray &secret() const;

  protected:
    QString _name;
    QString _device;
    QByteArray _secret;
  };

public:
  DeviceSettings();

  bool hasDevices() const;
  bool hasDevice(const QString &name) const;
  Device device(const QString &name) const;
  void add(const QString &name, const QString &device, const QByteArray &secret);

protected:
  QHash<QString, Device> _devices;
};


int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  app.setOrganizationDomain("hmatuschek.github.io");
  app.setApplicationName("dawncmd");

  Parser parser;
  RuleInterface &new_device = (parser.Keyword("new-device") , parser.Option("name"),
      parser.Option("device"), parser.Option("secret"));
  RuleInterface &device_info = (parser.Keyword("info"), parser.Value("devname"));
  parser.setGrammar(new_device | device_info);

  Logger::get().addHandler(new StreamLogHandler(LOG_DEBUG, std::cerr));

  if (! parser.parse((const char **)argv, argc)) {
    std::cout << "Usage " << parser.format_help("dawncmd") << std::endl;
    return -1;
  }

  if (parser.has_keyword("new-device")) {
    QString name = parser.get_option("name").front().c_str();
    QString device = parser.get_option("device").front().c_str();
    QByteArray secret = QByteArray::fromHex(parser.get_option("secret").front().c_str());

    DeviceSettings devices;
    devices.add(name, device, secret);

  } else if (parser.has_keyword("info")) {
    DeviceSettings devices;
    if (! devices.hasDevices()) {
      std::cerr << "No device configured. Call 'dawncmd new-device ...'." << std::endl;
      return -1;
    }
    QString device_name = parser.get_values("devname").front().c_str();
    if (! devices.hasDevice(device_name)) {
      std::cerr << "No device named '" << device_name.toStdString() << "' known." << std::endl;
      return -1;
    }

    Dawn dawn(devices.device(device_name).device(),
              (const uint8_t *)devices.device(device_name).secret().data());

  }

  return 0;
}


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
