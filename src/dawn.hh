#ifndef DAWN_HH
#define DAWN_HH

#include <QAbstractListModel>
#include <QSerialPort>
#include <QVector>
#include <QTime>
#include <QByteArray>

/** Proxy to the hardware. */
class Dawn : public QAbstractTableModel
{
  Q_OBJECT

public:
  /** Defines the possible alarm selection. */
  typedef enum {
    EVERYDAY, SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY
  } DayOfWeek;

  /** Represents an alarm configuration. */
  typedef struct {
    bool enabled;
    DayOfWeek dayOfWeek;
    QTime time;
  } Alarm;


public:
  /** Constructor, @c portname specifies the name of the serial port interfacing the
   * device. */
  explicit Dawn(const QString &portname, QObject *parent = 0);

  /** Returns the number of possible alarm configurations. */
  size_t numAlarms() const;
  /** Retunrs a particular alarm config. */
  const Alarm &alarm(size_t idx) const;
  /** (Re-) Sets the specified alarm. */
  bool setAlarm(size_t idx, const Alarm &alarm);

  /** Returns the current value (brightness) of the lamp. */
  uint8_t value();
  /** Resets the current value (brightness) of the lamp. */
  bool setValue(uint8_t value);

  /** Retunrs the time and date of the device. */
  QDateTime time();
  /** Sets the time & date of the device to the host date/time. */
  bool setTime();
  /** Sets the time & date of the device to the given date/time. */
  bool setTime(const QDateTime &time);

  /** Returns the current maximum brightness during a "dawn". */
  uint8_t maxValue();
  /** (Re-) Sets the maximum brightness during a dawn. */
  bool setMaxValue(uint8_t value);

  /** Returns the current "dawn" duration in minutes. */
  uint8_t duration();
  /** (Re-) Sets the "dawn" duration in minuted. */
  bool setDuration(uint8_t dur);

  /* Implementation of QAbstractListModel interface. */
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);

protected:
  bool _write(uint8_t c);
  bool _write(uint8_t *buffer, size_t len);

  bool _read(uint8_t &c);
  bool _read(uint8_t *buffer, size_t len);

  void _sign(uint8_t *buffer, size_t len);
  void _sign(uint8_t *buffer, size_t len, uint8_t *hash);

  bool _verify(uint8_t *buffer, size_t len);
  bool _verify(uint8_t *buffer, size_t len, uint8_t *sig);

  bool _send(uint8_t *cmd, size_t cmd_len, uint8_t *resp, size_t resp_len);

protected:
  /** The port interfacing the lamp. */
  QSerialPort _port;
  /** The list of alarms. */
  QVector<Alarm> _alarms;
  /** The secret shared with the device, initially 128 0-bits. */
  uint8_t _secret[16];
  /** The current salt (obtained from the device). */
  uint8_t _salt[8];
};

#endif // DAWN_HH
