#include "dawn.hh"
#include <QTextStream>
#include <iostream>


Dawn::Dawn(const QString &portname, QObject *parent)
  : QAbstractTableModel(parent), _port(portname)
{
  _port.setBaudRate(9600);
  _port.open(QIODevice::ReadWrite);

  bool ok;
  // Get alarms
  size_t nalarms = command("NALARM").toUInt(&ok);
  if (! ok) { nalarms = 0; }

  _alarms.resize(nalarms);
  for (size_t i=0; i<nalarms; i++) {
    // Get i-th alarm setting
    QString response = command(QString("ALARM %1").arg(i));
    int enabled, dayofweek, hour, minute;
    QTextStream(&response) >> enabled >> dayofweek >> hour >> minute;
    // Check for consistency
    if ((dayofweek < 0) || (dayofweek>7)) { enabled = false; dayofweek=0; }
    if ((hour < 0) || (hour > 23)) { enabled = false; hour=0; }
    if ((minute < 0) || (minute > 59)) { enabled = false; minute=0; }
    // Add to list of alarms
    _alarms[i].enabled = enabled; _alarms[i].dayOfWeek = DayOfWeek(dayofweek);
    _alarms[i].time = QTime(hour, minute);
  }
}

size_t
Dawn::numAlarms() const {
  return _alarms.size();
}

const Dawn::Alarm &
Dawn::alarm(size_t idx) const {
  return _alarms[idx];
}

bool
Dawn::setAlarm(size_t idx, const Alarm &alarm) {
  // Try to set alarm on device
  QString line = QString("SETALARM %1 %2 %3 %4 %5").arg(idx).arg(int(alarm.enabled));
  line = line.arg(int(alarm.dayOfWeek)).arg(alarm.time.hour()).arg(alarm.time.minute());
  if ("OK" != command(line)) { return false; }
  _alarms[idx] = alarm;
  return true;
}

uint8_t
Dawn::value() {
  return command("VALUE").toUShort();
}

bool
Dawn::setValue(uint8_t value) {
  return "OK" == command(QString("SETVALUE %1").arg(value));
}

QDateTime
Dawn::time() {
  return QDateTime::fromString(
        command("TIME"), "yyyy-M-d h:m:s");
}

bool
Dawn::setTime() {
  return setTime(QDateTime::currentDateTime());
}

bool
Dawn::setTime(const QDateTime &time) {
  return "OK" == command(
        QString("SETTIME %1").arg(
          time.toString("yyyy-M-d h:m:s")));
}

uint8_t
Dawn::maxValue() {
  return command("MAX").toUInt();
}

bool
Dawn::setMaxValue(uint8_t value) {
  return "OK" == command(
        QString("SETMAX %1").arg(value));
}

uint8_t
Dawn::duration() {
  return command("DUR").toUInt();
}

bool
Dawn::setDuration(uint8_t dur) {
  return "OK" == command(QString("SETDUR %1").arg(dur));
}

int
Dawn::rowCount(const QModelIndex &parent) const {
  return numAlarms();
}

int
Dawn::columnCount(const QModelIndex &parent) const {
  return 3;
}

Qt::ItemFlags
Dawn::flags(const QModelIndex &index) const {
  if (0 == index.column()) { return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable; }
  if (1 == index.column()) { return Qt::ItemIsEnabled | Qt::ItemIsEditable; }
  if (2 == index.column()) { return Qt::ItemIsEnabled | Qt::ItemIsEditable; }
  return Qt::ItemFlags();
}

QVariant
Dawn::data(const QModelIndex &index, int role) const
{
  if (index.row() >= numAlarms()) { QVariant(); }
  // Get alarm config
  Alarm alarm = this->alarm(index.row());
  // Dispatch
  if (Qt::DisplayRole == role) {
    if (1 == index.column()) {
      switch (alarm.dayOfWeek) {
      case Dawn::EVERYDAY:  return tr("Every day");
      case Dawn::SUNDAY:    return tr("Sunday");
      case Dawn::MONDAY:    return tr("Monday");
      case Dawn::TUESDAY:   return tr("Tuesday");
      case Dawn::WEDNESDAY: return tr("Wednesday");
      case Dawn::THURSDAY:  return tr("Thursday");
      case Dawn::FRIDAY:    return tr("Friday");
      case Dawn::SATURDAY:  return tr("Saturday");
      }
    }
    if (2 == index.column()) {
      return alarm.time.toString();
    }
  } else if (Qt::EditRole == role) {
    if (0 == index.column()) { return alarm.enabled; }
    if (1 == index.column()) { return int(alarm.dayOfWeek); }
    if (2 == index.column()) { return alarm.time; }
  } else if (Qt::CheckStateRole == role) {
    if (0 == index.column()) {
      return alarm.enabled ? Qt::Checked : Qt::Unchecked;
    }
  }
  return QVariant();
}

bool
Dawn::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (index.column() > 2) { return false; }
  if (index.row() >= numAlarms()) { return false; }
  // Get current config
  Alarm alarm = this->alarm(index.row());
  // Update config
  if (Qt::EditRole == role) {
    if (1 == index.column()) { alarm.dayOfWeek = DayOfWeek(value.toUInt()); }
    if (2 == index.column()) { alarm.time = value.toTime(); }
  } else if (Qt::CheckStateRole == role) {
    if (0 == index.column()) { alarm.enabled = value.toBool(); }
  } else {
    return false;
  }
  // Write to device
  bool success = setAlarm(index.row(), alarm);
  if (success) { emit dataChanged(index, index); }
  else { std::cerr << "Could not set alarm" << std::endl; }
  return success;
}

QString
Dawn::command(const QString &cmd) {
  QString response;
  if (! command(cmd, response)) { return ""; }
  return response;
}

bool
Dawn::command(const QString &cmd, QString &response)
{
  // Clear response variable
  response.clear();
  std::cerr << "CMD: " << cmd.toStdString() << std::endl;
  // Send command
  _port.write(cmd.toLocal8Bit()); _port.write("\n");
  // Wait for command to be send
  if (! _port.waitForBytesWritten(1000)) { return false; }
  // Read response
  if ( (!_port.bytesAvailable()) && (! _port.waitForReadyRead(1000)) ) { return false; }
  char c=0; _port.getChar(&c);
  while (c != '\n') {
    response.append(c);
    if ( (!_port.bytesAvailable()) && (! _port.waitForReadyRead(1000)) ) { return false; }
    _port.getChar(&c);
  }
  response = response.simplified();
  std::cerr << "RESP: '" << response.toStdString()
            << "' (" << response.length() << ")" << std::endl;
  return true;
}
