#include "dawn.hh"

#include <QStringList>
#include <QTextStream>
#include <QtEndian>

#include <iostream>
#include <inttypes.h>
#include <unistd.h>

#include "siphash24.h"
#include "logger.hh"

#define R25  1e3
#define a25  -0.041
#define Rr   1e2


typedef enum {
  GET_VALUE    = 0x01,
  SET_VALUE    = 0x02,
  GET_TIME     = 0x03,
  SET_TIME     = 0x04,
  GET_ALARM    = 0x05,
  SET_ALARM    = 0x06,
  GET_TEMP     = 0x07,
  GET_NALARM   = 0x08,
  GET_NONCE    = 0x09
} Commands;


Dawn::Dawn(QSerialPort *port, const unsigned char *secret, bool initAlarm, QObject *parent)
  : QObject(parent), _port(port), _valid(true)
{
  _port->setParent(this);

  // Store shared secret
  memcpy(_secret, secret, 16);

  _logmessages = new LogMessageTable();
  Logger::get().addHandler(_logmessages);

  // First, flush the port etc...
  if (!_recover()) {
    LogMessage msg(LOG_ERROR);
    msg << "IO: Can not read nonce from device.";
    Logger::get().log(msg);
    return;
  }

  // get number of alarm settings
  size_t nAlarm = readNumAlarms(&_valid);
  if (! _valid) { return; }

  _alarms.resize(nAlarm);
  if (initAlarm) {
    for (size_t i=0; i<nAlarm; i++) {
      bool ok = true;
      for (int j=0; j<5; j++) {
        loadAlarm(i, &ok);
        if (ok) { break; }
        usleep(100000);
      }
      if (! ok) { _valid = false; return;}
    }
  }
}

bool
Dawn::isValid() const {
  return _valid;
}

size_t
Dawn::numAlarms() const {
  return _alarms.size();
}

size_t
Dawn::readNumAlarms(bool *ok) {
  // Get number of alarm settings
  uint8_t tx_buffer[1] = { GET_NALARM }, rx_buffer[1];
  if (! _send(tx_buffer, 1, rx_buffer, 1)) {
    LogMessage msg(LOG_ERROR);
    msg << "Can not get number of alarm settings: Command failed.";
    Logger::get().log(msg);
    if (ok) { *ok = false; }
    return 0;
  }
  if (ok) { *ok = true; }
  return rx_buffer[0];
}

const Dawn::Alarm &
Dawn::alarm(size_t idx) const {
  return _alarms[idx];
}

bool
Dawn::setAlarm(size_t idx, const Alarm &alarm) {
  // Try to set alarm on device
  uint8_t tx_buffer[5] = { SET_ALARM, idx, alarm.dowFlags, alarm.time.hour(), alarm.time.minute() };
  if (! _send(tx_buffer, 5)) {
    LogMessage msg(LOG_WARNING);
    msg << "Can not set alarm: Command faild.";
    Logger::get().log(msg);
    return false;
  }
  _alarms[idx] = alarm;
  return true;
}

const Dawn::Alarm &
Dawn::loadAlarm(size_t i, bool *ok) {
  // Get i-th alarm setting
  uint8_t tx_buffer[2] = { GET_ALARM, (uint8_t) i }, rx_buffer[3];
  if (! _send(tx_buffer, 2, rx_buffer, 3)) {
    LogMessage msg(LOG_ERROR);
    msg << "Can not get alarm setting (idx="
        << i << "): Command failed.";
    Logger::get().log(msg);
    if (ok) { *ok = false; }
    _alarms[i].dowFlags = 0;
    _alarms[i].time = QTime();
  } else {
    uint8_t dayofweek = (rx_buffer[0] & 0x7f);
    uint8_t hour      = rx_buffer[1];
    uint8_t minute    = rx_buffer[2];
    // Check for consistency
    if ((hour > 23) || (minute > 59) || (dayofweek & 0x80)) {
      dayofweek = 0; hour=0; minute=0;
    }
    // Add to list of alarms
    _alarms[i].dowFlags = dayofweek;
    _alarms[i].time = QTime(hour, minute);
    *ok = true;
  }
  return _alarms[i];
}

uint16_t
Dawn::value(bool *ok) {
  uint8_t tx[1], rx[2];
  tx[0] = GET_VALUE;
  if (! _send(tx, 1, rx, 2)) {
    LogMessage msg(LOG_WARNING);
    msg << "Can not get value: Command faild.";
    Logger::get().log(msg);
    if (ok) { *ok = false; }
    return 0;
  }
  if (ok) { *ok = true; }
  return (uint16_t(rx[0])<<8) | rx[1];
}

bool
Dawn::setValue(uint16_t value) {
  uint8_t tx[3] = {SET_VALUE, value, value>>8};
  if (! _send(tx, 3)) {
    LogMessage msg(LOG_WARNING);
    msg << "Can not get value ("
        << value << "): Command faild.";
    Logger::get().log(msg);
    return false;
  }
  return true;
}

QDateTime
Dawn::time(bool *ok) {
  // Allocat buffers
  uint8_t  tx[1] = {GET_TIME}, rx[7];
  // send/receive
  if (! _send(tx, 1, rx, 7)) {
    LogMessage msg(LOG_WARNING);
    msg << "Can not get current time: Command failed.";
    Logger::get().log(msg);
    if (ok) { *ok = false; }
    return QDateTime();
  }
  if (ok) { *ok = true; }
  return QDateTime(QDate(2000 + rx[0], rx[1], rx[2]),
      QTime(rx[4], rx[5], rx[6]));
}

bool
Dawn::setTime() {
  return setTime(QDateTime::currentDateTime());
}

bool
Dawn::setTime(const QDateTime &time) {
  uint8_t tx[9] = {
    SET_TIME, std::max(0, time.date().year()-2000), time.date().month(), time.date().day(),
    time.date().dayOfWeek(), time.time().hour(), time.time().minute(), time.time().second()
  };
  return _send(tx, 8);
}

bool
Dawn::getTemp(double &core, double &amb) {
  uint8_t tx[1] = {GET_TEMP}, rx[4];
  if (! _send(tx, 1, rx, 4)) {
    LogMessage msg(LOG_WARNING);
    msg << "Can not read temperature: Command failed.";
    Logger::get().log(msg);
    return false;
  }
  core = qFromBigEndian(*((uint16_t *)rx));
  core = (core-275.)/1.5;

  amb  = qFromBigEndian(*((uint16_t *)(rx+2)));
  amb = Rr*(1024/amb - 1)/(R25*a25) + 25;
  return true;
}

bool
Dawn::readNonce() {
  // Get nonce from device
  uint8_t tx_buffer[1] = { GET_NONCE }, rx_buffer[8];
  if (! _send(tx_buffer, 1, rx_buffer, 8)) {
    LogMessage msg(LOG_ERROR);
    msg << "Can not get nonce: Command failed.";
    Logger::get().log(msg);
    return false;
  }
  memcpy(_nonce, rx_buffer, 8);
  return true;
}

LogMessageTable *
Dawn::logMessages() {
  return _logmessages;
}

bool
Dawn::_write(uint8_t c) {
  bool res = _port->putChar(c);
  return res;
}

bool
Dawn::_write(uint8_t *buffer, size_t len) {
  bool res = (len == size_t(_port->write((char *)buffer, len)));
  return res;
}

bool
Dawn::_read(uint8_t &c) {
  if (! _port->waitForReadyRead(1500)) {
    LogMessage msg(LOG_WARNING);
    msg << "read(): Timeout.";
    Logger::get().log(msg);
    return false;
  }
  return _port->getChar((char *) &c);
}

bool
Dawn::_read(uint8_t *buffer, size_t len) {
  size_t rem = len;
  while (rem) {
    if (! _port->waitForReadyRead(1500) ) {
      LogMessage msg(LOG_WARNING);
      msg << "read(): Timeout.";
      Logger::get().log(msg);
      return false;
    }
    int got = _port->read((char *)buffer, rem);
    if (-1 == got) {
      LogMessage msg(LOG_WARNING);
      msg <<"read(): Can not read from device.";
      Logger::get().log(msg);
      return false;
    }
    rem -= got;
  }
  return true;
}

void
Dawn::_sign(uint8_t *buffer, size_t len) {
  _sign(buffer, len, buffer+len);
}

void
Dawn::_sign(uint8_t *buffer, size_t len, uint8_t *sig) {
  for (size_t i=0; i<8; i++) { sig[0] = 0; }
  siphash24_cbc_mac(sig, buffer, len, _nonce, _secret);
}

bool
Dawn::_send(uint8_t *cmd, size_t cmd_len) {
  return _send(cmd, cmd_len, 0, 0);
}

bool
Dawn::_send(uint8_t *cmd, size_t cmd_len, uint8_t *resp, size_t resp_len) {
  uint8_t tx_buffer[cmd_len+8];
  memcpy(tx_buffer, cmd, cmd_len);
  _sign(tx_buffer, cmd_len, tx_buffer+cmd_len);

  // Send assembled and signed command
  if (! _write(tx_buffer, cmd_len+8)) {
    LogMessage msg(LOG_WARNING);
    msg << "send(): Can not send command.";
    Logger::get().log(msg);
    return false;
  }

  // Read response code
  uint8_t resp_code;
  if (! _read(&resp_code, 1)) {
    LogMessage msg(LOG_WARNING);
    msg << "send(): Can not read response-code.";
    return false;
  }

  // check response code
  if (resp_code) {
    LogMessage msg(LOG_WARNING);
    msg << "send(): Device returned error code (" << std::hex << int(resp_code) << ")";
    Logger::get().log(msg);
    _recover();
    return false;
  }

  // Read response (if any)
  if (resp_len) {
    if (! _read(resp, resp_len)) {
      LogMessage msg(LOG_WARNING);
      msg << "send(): Can not read response";
      Logger::get().log(msg);
      return false;
    }
  }

  {
    LogMessage msg(LOG_DEBUG);
    msg << "send(): Succcess";
    if (resp_len) {
      msg << " (" << resp_len << "): ";
      for (size_t i=0; i<resp_len; i++) {
        msg << " " << std::hex << int(resp[i]);
      }
    } else {
      msg << ".";
    }
    Logger::get().log(msg);
  }

  // Update nonce
  siphash24_cbc_update_nonce(_nonce, tx_buffer+cmd_len, _secret);
  return true;
}


bool
Dawn::_recover() {
  LogMessage msg(LOG_INFO); msg << "IO: Recover."; Logger::get().log(msg);
  _port->flush();
  // Wait a short time
  usleep(100000);
  _port->clear(QSerialPort::AllDirections);
  // try to read nonce
  return readNonce();
}
