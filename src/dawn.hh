#ifndef DAWN_HH
#define DAWN_HH

#include <QAbstractListModel>
#include <QSerialPort>
#include <QVector>
#include <QTime>

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
  /** The port interfacing the lamp. */
  QSerialPort _port;
  /** The list of alarms. */
  QVector<Alarm> _alarms;
};

#endif // DAWN_HH
