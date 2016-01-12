#ifndef CONFIGDIALOG_HH
#define CONFIGDIALOG_HH

#include <QDialog>
#include <QDateTimeEdit>
#include <QSlider>
#include <QSpinBox>
#include <QTimer>
#include <QLabel>

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
  QLabel *_coreTemp;
  QLabel *_ambTemp;
  double _coreAvg;
  double _ambAvg;
  QTimer _timer;
};

#endif // CONFIGDIALOG_HH
