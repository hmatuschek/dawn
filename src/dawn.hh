#ifndef DAWN_HH
#define DAWN_HH

#include <QAbstractListModel>
#include <QSerialPort>
#include <QVector>
#include <QTime>


class Dawn : public QAbstractListModel
{
  Q_OBJECT

public:
  typedef enum {
    EVERYDAY, SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY
  } DayOfWeek;

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

  /* Implementation of QAbstractListModel interface. */
  int rowCount(const QModelIndex &parent) const;
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
