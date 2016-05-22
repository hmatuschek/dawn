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


QString alarm2string(const Dawn::Alarm &alarm) {
  QStringList days;
  if (0b1111111 == alarm.dowFlags) { days.clear(); days << "Every day"; }
  else if (0b1000001 == alarm.dowFlags) { days.clear(); days << "Weekend"; }
  else if (0b0111110 == alarm.dowFlags) { days.clear(); days << "Work day"; }
  else {
    if (Dawn::MONDAY == (alarm.dowFlags & Dawn::MONDAY)) { days << "Monday"; }
    if (Dawn::TUESDAY == (alarm.dowFlags & Dawn::TUESDAY)) { days << "Tuesday"; }
    if (Dawn::WEDNESDAY == (alarm.dowFlags & Dawn::WEDNESDAY)) { days << "Wednesday"; }
    if (Dawn::THURSDAY == (alarm.dowFlags & Dawn::THURSDAY)) { days << "Thursday"; }
    if (Dawn::FRIDAY == (alarm.dowFlags & Dawn::FRIDAY)) { days << "Friday"; }
    if (Dawn::SATURDAY == (alarm.dowFlags & Dawn::SATURDAY)) { days << "Saturday"; }
    if (Dawn::SUNDAY == (alarm.dowFlags & Dawn::SUNDAY)) { days << "Sunday"; }
  }

  if (days.size()==1) {
    return QString("%0 at %1").arg(days.first(), alarm.time.toString("HH:mm"));
  } else if (days.size()){
    return QString("%0 and %1 at %2").arg(
          QStringList(days.mid(0, days.size()-1)).join(", "), days.last(), alarm.time.toString("HH:mm"));
  } else {
    return QString("Disabled");
  }
}


/* ********************************************************************************************* *
 * MAIN
 * ********************************************************************************************* */
int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);
  app.setOrganizationDomain("hmatuschek.github.io");
  app.setApplicationName("dawncmd");

  Parser parser;
  RuleInterface &new_device = (parser.Keyword("new-device") , parser.Option("name"),
      parser.Option("device"), parser.Option("secret"));
  RuleInterface &device_info = (parser.Keyword("info"), parser.Value("devname"));
  RuleInterface &on_off_cmd = ((parser.Keyword("on") | parser.Keyword("off")), parser.Value("devname"));
  parser.setGrammar(new_device | device_info | on_off_cmd);

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
  } else if (parser.has_keyword("on")) {
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

    if (! dawn.isValid()) {
      std::cerr << "Failed to access device." << std::endl;
      return -1;
    }

    dawn.setValue(0xffff);
  } else if (parser.has_keyword("off")) {
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

    if (! dawn.isValid()) {
      std::cerr << "Failed to access device." << std::endl;
      return -1;
    }

    dawn.setValue(0);
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

    if (! dawn.isValid()) {
      std::cerr << "Failed to access device." << std::endl;
      return -1;
    }

    double core, amb;
    if(! dawn.getTemp(core, amb)) {
      std::cerr << "Failed to get device temperatures." << std::endl;
      return -1;
    }

    QDateTime time = dawn.time();
    if (! time.isValid()) {
      std::cerr << "Failed to get device time." << std::endl;
      return -1;
    }

    std::cout << "Device info for '" << device_name.toStdString() << "'" << std::endl
              << " port: " << devices.device(device_name).device().toStdString() << std::endl
              << " temperature (core/amb): " << core << "/" << amb << std::endl
              << " device time: " << time.toString().toStdString() << std::endl
              << " alarm settings:" << std::endl;
    for (size_t i=0; i<dawn.numAlarms(); i++) {
      std::cout << "  " << i << ": " << alarm2string(dawn.alarm(i)).toStdString() << std::endl;
    }
  }

  return 0;
}



/* ********************************************************************************************* *
 * Implementation of DeviceSettings
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
