#include "dawn.hh"
#include <QStringList>
#include <QTextStream>
#include <iostream>
#include "siphash24.h"

typedef enum {
  GET_VALUE    = 0x01,
  SET_VALUE    = 0x02,
  GET_TIME     = 0x03,
  SET_TIME     = 0x04,
  GET_ALARM    = 0x05,
  SET_ALARM    = 0x06,
} Commands;


Dawn::Dawn(const QString &portname, const unsigned char *secret, QObject *parent)
  : QAbstractTableModel(parent), _port(portname), _valid(false)
{
  // Store shared secret
  memcpy(_secret, secret, 16);

  // Open port
  _port.setBaudRate(9600);
  _port.open(QIODevice::ReadWrite);
  if (! _port.isOpen()) { return; }

  // First, get current value
  uint8_t tx_buffer[32];
  uint8_t rx_buffer[32];
  tx_buffer[0] = GET_VALUE;
  if (! _send(tx_buffer, 1, rx_buffer, 1)) { return; }

  _alarms.resize(7);
  for (size_t i=0; i<7; i++) {
    // Get i-th alarm setting
    tx_buffer[0] = GET_ALARM; tx_buffer[1] = i;
    if (! _send(tx_buffer, 2, rx_buffer, 3)) { return; }
    uint8_t dayofweek = (rx_buffer[1] & 0x7f);
    uint8_t hour      = rx_buffer[2];
    uint8_t minute    = rx_buffer[3];
    // Check for consistency
    if ((hour < 0) || (hour > 23)) { dayofweek = 0; hour=0; }
    if ((minute < 0) || (minute > 59)) { dayofweek = 0; minute=0; }
    if (0 != (dayofweek & 0x80)) { dayofweek = 0; }
    // Add to list of alarms
    _alarms[i].dowFlags = dayofweek;
    _alarms[i].time = QTime(hour, minute);
  }

  _valid = true;
}

bool
Dawn::isValid() const {
  return _valid;
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
  uint8_t tx_buffer[5], rx_buffer[1];
  tx_buffer[0] = SET_ALARM;
  tx_buffer[1] = idx;
  tx_buffer[2] = alarm.dowFlags;
  tx_buffer[3] = alarm.time.hour();
  tx_buffer[4] = alarm.time.minute();
  if (! _send(tx_buffer, 5, rx_buffer, 1)) { return false; }
  _alarms[idx] = alarm;
  return true;
}

uint16_t
Dawn::value() {
  uint8_t tx[1], rx[2];
  tx[0] = GET_VALUE;
  if (! _send(tx, 1, rx, 2)) { return false; }
  return (uint16_t(rx[0])<<8) | rx[1];
}

bool
Dawn::setValue(uint16_t value) {
  uint8_t tx[3], rx[1];
  tx[0] = SET_VALUE; tx[1] = (value>>8); tx[2] = (value & 0xff);
  return _send(tx, 3, rx, 1);
}

QDateTime
Dawn::time() {
  uint8_t tx[1], rx[8];
  tx[0] = GET_TIME;
  if (! _send(tx, 1, rx, 8)) { return QDateTime(); }
  return QDateTime(QDate( *((uint16_t *)(rx)), rx[2], rx[3]),
      QTime(rx[5], rx[6], rx[7]));
}

bool
Dawn::setTime() {
  return setTime(QDateTime::currentDateTime());
}

bool
Dawn::setTime(const QDateTime &time) {
  uint8_t tx[9], rx[1];
  tx[0] = SET_TIME;
  *((uint16_t *)(tx+1)) = time.date().year();
  tx[3] = time.date().month();
  tx[4] = time.date().day();
  tx[5] = time.date().dayOfWeek();
  tx[6] = time.time().hour();
  tx[7] = time.time().minute();
  tx[8] = time.time().second();
  return _send(tx, 9, rx, 1);
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
    if (0 == index.column()) {
      QStringList repr;
      if (Dawn::MONDAY == (alarm.dowFlags & Dawn::MONDAY)) { repr << tr("Monday"); }
      if (Dawn::TUESDAY == (alarm.dowFlags & Dawn::TUESDAY)) { repr << tr("Tuesday"); }
      if (Dawn::WEDNESDAY == (alarm.dowFlags & Dawn::WEDNESDAY)) { repr << tr("Wednesday"); }
      if (Dawn::THURSDAY == (alarm.dowFlags & Dawn::THURSDAY)) { repr << tr("Thursday"); }
      if (Dawn::FRIDAY == (alarm.dowFlags & Dawn::FRIDAY)) { repr << tr("Friday"); }
      if (Dawn::SATURDAY == (alarm.dowFlags & Dawn::SATURDAY)) { repr << tr("Saturday"); }
      if (Dawn::SUNDAY == (alarm.dowFlags & Dawn::SUNDAY)) { repr << tr("Sunday"); }
      if (0b1111111 == alarm.dowFlags) { repr.clear(); repr << tr("Every day"); }
      if (0b1000001 == alarm.dowFlags) { repr.clear(); repr << tr("Weekend"); }
      if (0b0111110 == alarm.dowFlags) { repr.clear(); repr << tr("Work day"); }
    }
    if (1 == index.column()) {
      return alarm.time.toString();
    }
  } else if (Qt::EditRole == role) {
    if (0 == index.column()) { return int(alarm.dowFlags); }
    if (1 == index.column()) { return alarm.time; }
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
    if (0 == index.column()) { alarm.dowFlags = uint8_t(value.toUInt()); }
    if (1 == index.column()) { alarm.time = value.toTime(); }
  } else {
    return false;
  }
  // Write to device
  bool success = setAlarm(index.row(), alarm);
  if (success) { emit dataChanged(index, index); }
  else { std::cerr << "Could not set alarm" << std::endl; }
  return success;
}

bool
Dawn::_write(uint8_t c) {
  bool res = _port.putChar(c);
  _port.flush();
  return res;
}

bool
Dawn::_write(uint8_t *buffer, size_t len) {
  bool res = (len == _port.write((char *)buffer, len));
  _port.flush();
  return res;
}

bool
Dawn::_read(uint8_t &c) {
  if (! _port.bytesAvailable() && (! _port.waitForReadyRead(1000))) { return false; }
  return _port.getChar((char *) &c);
}

bool
Dawn::_read(uint8_t *buffer, size_t len) {
  size_t rem = len;
  while (rem) {
    if (! _port.bytesAvailable() && (! _port.waitForReadyRead(1000))) { return false; }
    int got = _port.read((char *)buffer, rem);
    if (-1 == got) { return false; }
    buffer += got; rem -= got;
  }
  return true;
}

void
Dawn::_sign(uint8_t *buffer, size_t len) {
  _sign(buffer, len, buffer+len);
}

void
Dawn::_sign(uint8_t *buffer, size_t len, uint8_t *sig) {
  memset(sig, 0, 8);
  siphash24_cbc_mac(sig, buffer, len, _secret);
}

bool
Dawn::_send(uint8_t *cmd, size_t cmd_len, uint8_t *resp, size_t resp_len) {
  uint8_t tx_buffer[cmd_len+8];
  memcpy(tx_buffer, cmd, cmd_len);
  _sign(tx_buffer, cmd_len);
  if (! _write(tx_buffer, cmd_len+8)) { return false; }
  if (! _read(resp, resp_len)) { return false; }
  return true;
}
