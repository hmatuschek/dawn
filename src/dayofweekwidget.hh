#ifndef DAYOFWEEKWIDGET_HH
#define DAYOFWEEKWIDGET_HH

#include <QListWidget>

class DayOfWeekWidget : public QListWidget
{
  Q_OBJECT

public:
  DayOfWeekWidget(uint8_t dow, QWidget *parent=0);

  uint8_t dayOfWeek() const;
  void setDayOfWeek(uint8_t dow);
};

#endif // DAYOFWEEKWIDGET_HH
