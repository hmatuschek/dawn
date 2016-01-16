#ifndef ALARMSETTINGWIDGET_HH
#define ALARMSETTINGWIDGET_HH

#include <QLabel>
#include <QDialog>

#include "dawn.hh"
#include "dayofweekwidget.hh"
#include <QTimeEdit>


/** A simple widget allowing to set a specific alarm setting. */
class AlarmSettingWidget : public QLabel
{
  Q_OBJECT

public:
  /** Constructor.
   * @param dawn Specifies the @c Dawn instance.
   * @param idx Specifies the i-th alarm setting. */
  explicit AlarmSettingWidget(Dawn &dawn, size_t idx, QWidget *parent = 0);


protected:
  /** On click. */
  void mouseReleaseEvent(QMouseEvent *ev);
  /** Updates the widget. */
  void updateText();

protected:
  /** A weak reference to the @c Dawn instance. */
  Dawn &_dawn;
  /** The index of the alarm setting. */
  size_t _idx;
};


/** A simple dialog to config a alarm setting. */
class AlarmSettingDialog: public QDialog
{
  Q_OBJECT

public:
  /** Constructor.
   * @param alarm specifies the alarm setting. */
  explicit AlarmSettingDialog(const Dawn::Alarm &alarm, QWidget *parent=0);
  /** Returns the current alarm setting. */
  Dawn::Alarm alarm() const;

protected:
  /** A widget to select the day of week of the alarm. */
  DayOfWeekWidget *_dow;
  /** A widget to select the alarm time. */
  QTimeEdit *_time;
};


/** A widget showing all alarm settings of the device. */
class AlarmSettingsWidget: public QWidget
{
  Q_OBJECT

public:
  /** Constructor.
   * @param dawn The @c Dawn instance. */
  explicit AlarmSettingsWidget(Dawn &dawn, QWidget *parent=0);

protected:
  /** A weak reference to the dawn instance. */
  Dawn &_dawn;
};



#endif // ALARMSETTINGWIDGET_HH
