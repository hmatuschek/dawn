#include "dawn.hh"
#include <QTextStream>
#include <iostream>
#include "siphash24.h"

typedef enum {
  GET_SALT     = 0x00,
  GET_VALUE    = 0x01,
  SET_VALUE    = 0x02,
  GET_TIME     = 0x03,
  SET_TIME     = 0x04,
  GET_N_ALARMS = 0x05,
  GET_ALARM    = 0x06,
  SET_ALARM    = 0x07,
  GET_MAX_DIM  = 0x08,
  SET_MAX_DIM  = 0x09,
  GET_DIM_DUR  = 0x0A,
  SET_DIM_DUR  = 0x0B
} Commands;

typedef enum {
  RESP_SUCCESS = 0x00,
  RESP_CMD_UNKNOWN = 0x01,
  RESP_SIG_INVALID = 0x02,
  RESP_CMD_ERROR = 0x03
} Responses;


Dawn::Dawn(const QString &portname, QObject *parent)
  : QAbstractTableModel(parent), _port(portname)
{
  _port.setBaudRate(9600);
  _port.open(QIODevice::ReadWrite);

  // First, get SALT
  if (! _write(GET_SALT)) { return; }
  if (! _read(_salt, 8)) { return; }

  // Get N alarms
  uint8_t tx_buffer[32];
  uint8_t rx_buffer[32];
  tx_buffer[0] = GET_N_ALARMS;
  if (! _send(tx_buffer, 1, rx_buffer, 2)) { return; }

  uint8_t nalarms = rx_buffer[1];
  _alarms.resize(nalarms);
  for (size_t i=0; i<nalarms; i++) {
    // Get i-th alarm setting
    tx_buffer[0] = GET_ALARM; tx_buffer[1] = i;
    _send(tx_buffer, 2, rx_buffer, 4);
    bool enabled  = rx_buffer[1] & 0x01;
    int dayofweek = (rx_buffer[1] >> 1) & 0x07;
    int hour      = rx_buffer[2];
    int minute    = rx_buffer[3];
    // Check for consistency
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
  uint8_t tx_buffer[5], rx_buffer[1];
  tx_buffer[0] = SET_ALARM;
  tx_buffer[1] = idx;
  tx_buffer[2] = (uint8_t(alarm.dayOfWeek)<<1) | (alarm.enabled & 0x01);
  tx_buffer[3] = alarm.time.hour();
  tx_buffer[4] = alarm.time.minute();
  if (! _send(tx_buffer, 5, rx_buffer, 1)) { return false; }
  _alarms[idx] = alarm;
  return true;
}

uint8_t
Dawn::value() {
  uint8_t tx[1], rx[2];
  tx[0] = GET_VALUE;
  if (! _send(tx, 1, rx, 2)) { return false; }
  return rx[1];
}

bool
Dawn::setValue(uint8_t value) {
  uint8_t tx[2], rx[1];
  tx[0] = SET_VALUE; tx[1] = value;
  return _send(tx, 2, rx, 1);
}

QDateTime
Dawn::time() {
  uint8_t tx[1], rx[8];
  tx[0] = GET_TIME;
  if (! _send(tx, 1, rx, 8)) { return QDateTime(); }
  return QDateTime(QDate( *((uint16_t *)(rx+1)), rx[3], rx[4]),
      QTime(rx[5], rx[6], rx[7]));
}

bool
Dawn::setTime() {
  return setTime(QDateTime::currentDateTime());
}

bool
Dawn::setTime(const QDateTime &time) {
  uint8_t tx[8], rx[1];
  tx[0] = SET_TIME;
  *((uint16_t *)(tx+1)) = time.date().year();
  tx[3] = time.date().month();
  tx[4] = time.date().day();
  tx[5] = time.time().hour();
  tx[6] = time.time().minute();
  tx[7] = time.time().second();
  return _send(tx, 8, rx, 1);
}

uint8_t
Dawn::maxValue() {
  uint8_t tx[1], rx[2];
  tx[0] = GET_MAX_DIM;
  if (! _send(tx, 1, rx, 2)) { return 0; }
  return rx[1];
}

bool
Dawn::setMaxValue(uint8_t value) {
  uint8_t tx[2], rx[1];
  tx[0] = SET_MAX_DIM; tx[1] = value;
  return _send(tx, 2, rx, 1);
}

uint8_t
Dawn::duration() {
  uint8_t tx[1], rx[2];
  tx[0] = GET_DIM_DUR;
  if (! _send(tx, 1, rx, 2)) { return 0; }
  return rx[1];
}

bool
Dawn::setDuration(uint8_t dur) {
  uint8_t tx[2], rx[1];
  tx[0] = SET_DIM_DUR; tx[1] = dur;
  return _send(tx, 2, rx, 1);
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
  memcpy(sig, _salt, 8);
  siphash24_cbc_mac(sig, buffer, len, _secret);
}

bool
Dawn::_verify(uint8_t *buffer, size_t len) {
  return _verify(buffer, len, buffer+len);
}

bool
Dawn::_verify(uint8_t *buffer, size_t len, uint8_t *sig) {
  uint8_t osign[8];
  _sign(buffer, len, osign);
  return 0==memcmp(osign, sig, 8);
}

bool
Dawn::_send(uint8_t *cmd, size_t cmd_len, uint8_t *resp, size_t resp_len) {
  uint8_t tx_buffer[cmd_len+8];
  memcpy(tx_buffer, cmd, cmd_len);
  _sign(tx_buffer, cmd_len);
  if (! _write(tx_buffer, cmd_len+8)) { return false; }

  uint8_t rx_buffer[resp_len+8];
  if (! _read(rx_buffer, 1)) { return false; }
  if (RESP_SUCCESS != rx_buffer[0]) { return false; }

  if (! _read(rx_buffer, resp_len+7)) { return false; }
  // Update IV
  memcpy(_salt, tx_buffer+cmd_len, 8);
  // Verify response
  if (! _verify(rx_buffer, resp_len)) { return false; }
  memcpy(resp, rx_buffer, resp_len);
  // Update IV
  memcpy(_salt, rx_buffer+resp_len, 8);
  return true;
}
