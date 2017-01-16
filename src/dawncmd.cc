#include "dawn.hh"
#include <iostream>
#include <QCoreApplication>
#include <QSettings>
#include <QString>

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

uint8_t string2dayofweek(const QString &str) {
  if (str.contains("all"))
    return 0b1111111;
  if (str.contains("none"))
    return 0;

  uint8_t mask = 0;
  if (str.contains("mon"))
    mask |= Dawn::MONDAY;
  if (str.contains("tue"))
    mask |= Dawn::TUESDAY;
  if (str.contains("wed"))
    mask |= Dawn::WEDNESDAY;
  if (str.contains("thu"))
    mask |= Dawn::THURSDAY;
  if (str.contains("fri"))
    mask |= Dawn::FRIDAY;
  if (str.contains("sat"))
    mask |= Dawn::SATURDAY;
  if (str.contains("sun"))
    mask |= Dawn::SUNDAY;
  if (str.contains("weekday"))
    mask |= 0b0111110;
  if (str.contains("weekend"))
    mask |= 0b1000001;
  return mask;
}



Dawn *
connect(const QString &devname, bool initAlarm=true) {
  DeviceSettings devices;
  if (! devices.hasDevices()) {
    std::cerr << "No device configured. Call 'dawncmd new-device ...'." << std::endl;
    return 0;
  }

  if (! devices.hasDevice(devname)) {
    std::cerr << "No device named '" << devname.toStdString() << "' known." << std::endl;
    return 0;
  }

  QString portname = devices.device(devname).device();
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

  Dawn *dawn = new Dawn(port, (const uint8_t *)devices.device(devname).secret().data(), initAlarm);
  if (! dawn->isValid()) {
    std::cerr << "Failed to access device." << std::endl;
    return 0;
  }

  return dawn;
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
  RuleInterface &set_alarm  = (parser.Keyword("setalarm"), parser.Value("num"), parser.Value("days"), parser.Value("time"), parser.Value("devname"));
  RuleInterface &set_time = (parser.Keyword("settime"), parser.Value("devname"));
  RuleInterface &scan = (parser.Keyword("scan"));
  RuleInterface &options  = (parser.Option("log"));
  parser.setGrammar( (parser.opt(options), (scan | new_device | device_info | on_off_cmd | set_alarm | set_time)) );

  if (! parser.parse((const char **)argv, argc)) {
    std::cout << "Usage " << parser.format_help("dawncmd") << std::endl;
    return -1;
  }

  if (parser.has_option("log") && (parser.get_option("log").front() == "debug"))
    Logger::get().addHandler(new StreamLogHandler(LOG_DEBUG, std::cerr));
  else if (parser.has_option("log") && (parser.get_option("log").front() == "info"))
    Logger::get().addHandler(new StreamLogHandler(LOG_INFO, std::cerr));
  else if (parser.has_option("log") && (parser.get_option("log").front() == "warn"))
    Logger::get().addHandler(new StreamLogHandler(LOG_WARNING, std::cerr));
  else
    Logger::get().addHandler(new StreamLogHandler(LOG_ERROR, std::cerr));

  if (parser.has_keyword("new-device")) {
    QString name = parser.get_option("name").front().c_str();
    QString device = parser.get_option("device").front().c_str();
    QByteArray secret = QByteArray::fromHex(parser.get_option("secret").front().c_str());

    DeviceSettings devices;
    devices.add(name, device, secret);
  } else if (parser.has_keyword("on")) {
    Dawn *dawn = connect(parser.get_values("devname").front().c_str(), false);
    if (0 == dawn) {
      std::cerr << "Failed to access device " << parser.get_values("devname").front() << "." << std::endl;
      return -1;
    }
    dawn->setValue(0xffff);
    delete dawn;
  } else if (parser.has_keyword("off")) {
    Dawn *dawn = connect(parser.get_values("devname").front().c_str(), false);
    if (0 == dawn)
      return -1;
    dawn->setValue(0);
    delete dawn;
  } else if (parser.has_keyword("info")) {
    Dawn *dawn = connect(parser.get_values("devname").front().c_str(), true);
    if (0 == dawn)
      return -1;

    double core, amb;
    if(! dawn->getTemp(core, amb)) {
      std::cerr << "Failed to get device temperatures." << std::endl;
      return -1;
    }

    QDateTime time = dawn->time();
    if (! time.isValid()) {
      std::cerr << "Failed to get device time." << std::endl;
      return -1;
    }

    std::cout << "Device info for '" << parser.get_values("devname").front() << "'" << std::endl
              << " temperature (core/amb): " << core << "/" << amb << std::endl
              << " device time: " << time.toString().toStdString() << std::endl
              << " alarm settings:" << std::endl;
    for (size_t i=0; i<dawn->numAlarms(); i++) {
      std::cout << "  " << i << ": " << alarm2string(dawn->alarm(i)).toStdString() << std::endl;
    }

    delete dawn;
  } else if (parser.has_keyword("setalarm")) {
    int num = atoi(parser.get_values("num").front().c_str());
    if (num<0) {
      std::cerr << "Invalid alarm index " << num << " must be >= 0." << std::endl;
      return -1;
    }

    Dawn::Alarm alarm;
    alarm.dowFlags = string2dayofweek(parser.get_values("days").front().c_str());
    alarm.time = QTime::fromString(parser.get_values("time").front().c_str(), "hh:mm");

    LogMessage msg(LOG_DEBUG);
    msg << "Set alarm " << num << " to " << alarm2string(alarm).toStdString() << ".";
    Logger::get().log(msg);

    Dawn *dawn = connect(parser.get_values("devname").front().c_str(), false);
    if (0 == dawn)
      return -1;

    while (! dawn->setAlarm(num, alarm)) { }

    delete dawn;
  } else if (parser.has_keyword("settime")) {
    Dawn *dawn = connect(parser.get_values("devname").front().c_str(), false);
    if (0 == dawn)
      return -1;
    while (! dawn->setTime()) { }
  } else if (parser.has_keyword("scan")) {
    DawnDiscover discover;
    if (! discover.start()) {
      std::cerr << "Oops..." <<std::endl;
      return -1;
    }
    app.exec();
  }

  return 0;
}

