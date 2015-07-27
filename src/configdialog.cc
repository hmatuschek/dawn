#include "configdialog.hh"
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>

ConfigDialog::ConfigDialog(Dawn &dawn, QWidget *parent) :
  QDialog(parent), _dawn(dawn)
{
  _dtEdit = new QDateTimeEdit(_dawn.time());
  _dtEdit->setCalendarPopup(true);
  QPushButton *now = new QPushButton(tr("now"));
  QHBoxLayout *dtLayout = new QHBoxLayout();
  dtLayout->addWidget(_dtEdit);
  dtLayout->addWidget(now);

  _coreTemp = new QLabel(tr("--- °C"));
  _ambTemp  = new QLabel(tr("--- °C"));

  QFormLayout *flayout = new QFormLayout();
  flayout->addRow(tr("Date and Time"), dtLayout);
  flayout->addRow(tr("CPU temperature"), _coreTemp);
  flayout->addRow(tr("Device temperature"), _ambTemp);
  this->setLayout(flayout);

  QObject::connect(now, SIGNAL(clicked()), this, SLOT(onSetTimeToNow()));
  //QObject::connect(_dtEdit, SIGNAL(editingFinished()), this, SLOT(onSetTime()));
  QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(onUpdateTime()));

  _timer.setInterval(5000);
  _timer.setSingleShot(false);
  _timer.start();
}

void
ConfigDialog::onSetTimeToNow() {
  _dtEdit->setDateTime(QDateTime::currentDateTime());
  _dawn.setTime(QDateTime::currentDateTime());
}

void
ConfigDialog::onSetTime() {
  _dawn.setTime(_dtEdit->dateTime());
}

void
ConfigDialog::onUpdateTime() {
  _dtEdit->setDateTime(_dawn.time());
  double coreTemp, ambTemp;
  if (_dawn.getTemp(coreTemp, ambTemp)) {
    _coreTemp->setText(tr("%0 °C").arg(coreTemp));
    _ambTemp->setText(tr("%0 °C").arg(ambTemp));
  }
}
