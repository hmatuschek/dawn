#ifndef CONFIGDIALOG_HH
#define CONFIGDIALOG_HH

#include <QDialog>
#include <QDateTimeEdit>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>

#include "dawn.hh"


class ConfigDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ConfigDialog(Dawn &dawn, QWidget *parent = 0);

protected slots:
  void onSetTimeToNow();
  void onSetTime();
  void onUpdateTime();

protected:
  Dawn &_dawn;
  QDateTimeEdit *_dtEdit;
  QTimer _timer;
};

#endif // CONFIGDIALOG_HH
