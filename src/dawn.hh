#ifndef DAWN_HH
#define DAWN_HH

#include <QObject>
#include <QSerialPort>
#include <QVector>
#include <QTime>
#include <QByteArray>

#include <inttypes.h>

#include "logmessagetable.hh"


/** Proxy to the hardware. */
class Dawn : public QObject
{
  Q_OBJECT

public:
  /** Defines the possible alarm selection. */
  typedef enum {
    SUNDAY    = 0b0000001,
    MONDAY    = 0b0000010,
    TUESDAY   = 0b0000100,
    WEDNESDAY = 0b0001000,
    THURSDAY  = 0b0010000,
    FRIDAY    = 0b0100000,
    SATURDAY  = 0b1000000,
  } DayOfWeek;

  /** Represents an alarm configuration. */
  typedef struct {
    uint8_t dowFlags;
    QTime time;
  } Alarm;


public:
  /** Constructor, @c portname specifies the name of the serial port interfacing the
   * device. */
  explicit Dawn(const QString &portname, const unsigned char *secret, QObject *parent = 0);

  /** Returns @c true, if the device was initialized correctly. */
  bool isValid() const;

  /** Returns the number of possible alarm configurations. */
  size_t numAlarms() const;
  /** Reads the number of possible alarm configurations from the device. */
  size_t readNumAlarms(bool *ok=0);
  /** Loads and returns the specified alarm. */
  const Alarm &loadAlarm(size_t idx, bool *ok=0);
  /** Retunrs a particular alarm config. */
  const Alarm &alarm(size_t idx) const;
  /** (Re-) Sets the specified alarm. */
  bool setAlarm(size_t idx, const Alarm &alarm);

  /** Returns the current value (brightness) of the lamp. */
  uint16_t value(bool *ok=0);
  /** Resets the current value (brightness) of the lamp. */
  bool setValue(uint16_t value);

  /** Retunrs the time and date of the device. */
  QDateTime time(bool *ok=0);
  /** Sets the time & date of the device to the host date/time. */
  bool setTime();
  /** Sets the time & date of the device to the given date/time. */
  bool setTime(const QDateTime &time);

  /** Reads the core and device temperature. */
  bool getTemp(double &core, double &amb);

  /** Reads the curren device nonce. */
  bool readNonce();

  LogMessageTable *logMessages();

protected:
  /** Sends a char to the device. */
  bool _write(uint8_t c);
  /** Sends some data to the device. */
  bool _write(uint8_t *buffer, size_t len);

  /** Receives a char from the device. */
  bool _read(uint8_t &c);
  /** Receives some data from the device. */
  bool _read(uint8_t *buffer, size_t len);
  /** Signs the first @c len bytes of the buffer.
   * The signature is appended to the buffer, hence it must be of at least size len+8. */
  void _sign(uint8_t *buffer, size_t len);
  /** Signs the first @c len bytes of the buffer and stores the signature into
   * hash. */
  void _sign(uint8_t *buffer, size_t len, uint8_t *hash);
  /** Sends the given command to the device. Expects no response. */
  bool _send(uint8_t *cmd, size_t cmd_len);
  /** Sends the given command to the device. The response is stored into @c resp. */
  bool _send(uint8_t *cmd, size_t cmd_len, uint8_t *resp, size_t resp_len);
  /** Recovers from an io-error condition. */
  bool _recover();

protected:
  /** The port interfacing the lamp. */
  QSerialPort _port;
  /** Log messages */
  LogMessageTable *_logmessages;
  /** Is set to @c true if constructed correctly. */
  bool _valid;
  /** The list of alarms. */
  QVector<Alarm> _alarms;
  /** The secret shared with the device, initially 128 0-bits. */
  uint8_t _secret[16];
  /** The current nonce (obtained from the device). */
  uint8_t _nonce[8];
};

#endif // DAWN_HH
