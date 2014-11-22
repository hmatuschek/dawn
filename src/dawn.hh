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
  explicit Dawn(const QString &portname, QObject *parent = 0);

  size_t numAlarms() const;
  const Alarm &alarm(size_t idx) const;
  bool setAlarm(size_t idx, const Alarm &alarm);

  uint8_t value();
  bool setValue(uint8_t value);

  QDateTime time();
  bool setTime();
  bool setTime(const QDateTime &time);

  /* Implementation of QAbstractListModel interface. */
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  //QVariant headerData(int section, Qt::Orientation orientation, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  bool setData(const QModelIndex &index, const QVariant &value, int role);

protected:
  /** The port interfacing the lamp. */
  QSerialPort _port;
  /** The list of alarms. */
  QVector<Alarm> _alarms;
};

#endif // DAWN_HH
