#include "dawn.hh"
#include <QTextStream>


Dawn::Dawn(const QString &portname, QObject *parent)
  : QAbstractListModel(parent), _port(portname)
{
  _port.setBaudRate(9600);
  _port.open(QIODevice::ReadWrite);

  bool ok;
  // Get alarms
  _port.write("NALARMS\n");
  size_t nalarms = _port.readLine().toUInt(&ok);
  if (! ok) { nalarms = 0; }
  _alarms.resize(nalarms);
  for (size_t i=0; i<nalarms; i++) {
    // Get i-th alarm setting
    _port.write(QString("ALARM %1\n").arg(i).toLocal8Bit());
    QString line = _port.readLine();
    int enabled, dayofweek, hour, minute;
    QTextStream(&line) >> enabled >> dayofweek >> hour >> minute;
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
  QString line = QString("SETALARM %1 %2 %3 %4 %5\n").arg(idx).arg(int(alarm.enabled));
  line = line.arg(int(alarm.dayOfWeek)).arg(alarm.time.hour()).arg(alarm.time.minute());
  _port.write(line.toLocal8Bit());
  // Check success
  if ("OK" != _port.readLine()) { return false; }
  _alarms[idx] = alarm;
  return true;
}

uint8_t
Dawn::value() {
  _port.write("VALUE\n");
  return _port.readLine().toUShort();
}

bool
Dawn::setValue(uint8_t value) {
  _port.write(QString("SETVALUE %1\n").arg(value).toLocal8Bit());
  if ("OK" != _port.readLine()) { return false; }
  return true;
}



int
Dawn::rowCount(const QModelIndex &parent) const {
  return _alarms.size();
}

Qt::ItemFlags
Dawn::flags(const QModelIndex &index) const {
  if (0 == index.column()) { return Qt::ItemIsEditable | Qt::ItemIsUserCheckable; }
  if (1 == index.column()) { return Qt::ItemIsEditable; }
  if (2 == index.column()) { return Qt::ItemIsEditable; }
  return Qt::ItemFlags();
}

QVariant
Dawn::data(const QModelIndex &index, int role) const {
  if (index.row() >= _alarms.size()) { QVariant(); }
  if (Qt::DisplayRole == role) {
    if (0 == index.column()) {
      if (_alarms[index.row()].enabled) { return tr("Enabled"); }
      return tr("Disabled");
    }
    if (1 == index.column()) {
      switch (_alarms[index.row()].dayOfWeek) {
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
      return _alarms[index.row()].time.toString();
    }
  } else if (Qt::EditRole == role) {
    if (0 == index.column()) { return _alarms[index.row()].enabled; }
    if (1 == index.column()) { return int(_alarms[index.row()].dayOfWeek); }
    if (2 == index.column()) { return _alarms[index.row()].time; }
  }
  return QVariant();
}

bool
Dawn::setData(const QModelIndex &index, const QVariant &value, int role) {
  if (role != Qt::EditRole) { return false; }
  if (index.column() > 2) { return false; }
  if (index.row() >= _alarms.size()) { return false; }
  if (0 == index.column()) { _alarms[index.row()].enabled = value.toBool(); }
  if (1 == index.column()) { _alarms[index.row()].dayOfWeek = DayOfWeek(value.toUInt()); }
  if (2 == index.column()) { _alarms[index.row()].time = value.toTime(); }
  return true;
}
