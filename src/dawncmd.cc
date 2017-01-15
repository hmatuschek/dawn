#include "dawn.hh"
#include <iostream>
#include <QCoreApplication>
#include <QSettings>
#include <QString>
#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothSocket>

#include "option_parser.hh"
#include "devicesettings.hh"
#include "dawndiscover.hh"
#include <QLoggingCategory>


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


QSerialPort *
connect(const QString &portname) {
  QSerialPort *port = new QSerialPort(portname);

  // Open port
  port->open(QIODevice::ReadWrite);
  if (! port->isOpen()) {
    LogMessage msg(LOG_ERROR);
    msg << "IO Error: Can not open device " << portname.toStdString();
    Logger::get().log(msg);
    return 0;
  }
  if (! port->setBaudRate(QSerialPort::Baud38400)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set baudrate.";
    Logger::get().log(msg);
    return 0;
  }
  if (! port->setDataBits(QSerialPort::Data8)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set data bits.";
    Logger::get().log(msg);
    return 0;
  }
  if (! port->setParity(QSerialPort::NoParity)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set parity.";
    Logger::get().log(msg);
    return 0;
  }
  if (! port->setStopBits(QSerialPort::OneStop)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set stop bits.";
    Logger::get().log(msg);
    return 0;
  }
  if (! port->setFlowControl(QSerialPort::HardwareControl)) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not set stop bits.";
    Logger::get().log(msg);
    return 0;
  }

  return port;
}

/* ********************************************************************************************* *
 * MAIN
 * ********************************************************************************************* */
int main(int argc, char *argv[])
{
  QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));

  QCoreApplication app(argc, argv);
  app.setOrganizationDomain("hmatuschek.github.io");
  app.setApplicationName("dawncmd");

  Parser parser;
  RuleInterface &new_device = (parser.Keyword("new-device") , parser.Option("name"),
      parser.Option("device"), parser.Option("secret"));
  RuleInterface &device_info = (parser.Keyword("info"), parser.Value("devname"));
  RuleInterface &on_off_cmd = ((parser.Keyword("on") | parser.Keyword("off")), parser.Value("devname"));
  RuleInterface &scan = (parser.Keyword("scan"), parser.Value("addr"));
  RuleInterface &con = (parser.Keyword("connect"), parser.Value("addr"));
  parser.setGrammar(con | scan | new_device | device_info | on_off_cmd);

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

    QSerialPort *port = connect(devices.device(device_name).device());
    if (0 == port) {
      std::cerr << "Failed to access device." << std::endl;
      return -1;
    }

    Dawn dawn(port, (const uint8_t *)devices.device(device_name).secret().data());
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

    QSerialPort *port = connect(devices.device(device_name).device());
    if (0 == port) {
      std::cerr << "Failed to access device." << std::endl;
      return -1;
    }

    Dawn dawn(port, (const uint8_t *)devices.device(device_name).secret().data());
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

    QSerialPort *port = connect(devices.device(device_name).device());
    if (0 == port) {
      std::cerr << "Failed to access device." << std::endl;
      return -1;
    }

    Dawn dawn(port, (const uint8_t *)devices.device(device_name).secret().data());
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
  } else if (parser.has_keyword("scan") && parser.has_values("addr")) {
    QList<QBluetoothHostInfo> devices = QBluetoothLocalDevice::allDevices();
    if (0 == devices.size()) {
      std::cerr << "No valid local BT devices found." << std::endl;
    } else {
      std::cerr << "Using " << devices[0].name().toStdString()
                << " @ " << devices[0].address().toString().toStdString() << std::endl;
    }
    DawnDiscover discover(QBluetoothAddress(parser.get_values("addr").front().c_str()));
    if (discover.start())
      app.exec();
  }

  return 0;
}

