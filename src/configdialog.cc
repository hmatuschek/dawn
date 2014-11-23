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

  _maxBright = new QSlider(Qt::Horizontal);
  _maxBright->setMinimum(0); _maxBright->setMaximum(255);
  _maxBright->setValue(_dawn.maxValue());

  _dawnDur = new QSpinBox();
  _dawnDur->setMinimum(0); _dawnDur->setMaximum(255);
  _dawnDur->setValue(_dawn.duration());

  _timer.setInterval(1000);
  _timer.setSingleShot(false);

  QFormLayout *flayout = new QFormLayout();
  flayout->addRow(tr("Date and Time"), dtLayout);
  flayout->addRow(tr("Brightness"), _maxBright);
  flayout->addRow(tr("Duration"), _dawnDur);

  this->setLayout(flayout);

  QObject::connect(now, SIGNAL(clicked()), this, SLOT(onSetTimeToNow()));
  QObject::connect(_dtEdit, SIGNAL(editingFinished()), this, SLOT(onSetTime()));
  QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(onUpdateTime()));
  QObject::connect(_maxBright, SIGNAL(valueChanged(int)), this, SLOT(onBrightnessChanged(int)));
  QObject::connect(_dawnDur, SIGNAL(valueChanged(int)), this, SLOT(onDurationChanged(int)));
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
}

void
ConfigDialog::onBrightnessChanged(int value) {
  value = std::max(0, std::min(value, 255));
  _dawn.setMaxValue(value);
  _dawn.setValue(value);
}

void
ConfigDialog::onDurationChanged(int value) {
  value = std::max(0, std::min(value, 255));
  _dawn.setDuration(value);
}
