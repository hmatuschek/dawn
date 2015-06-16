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

  _timer.setInterval(1000);
  _timer.setSingleShot(false);
  _timer.start();

  QFormLayout *flayout = new QFormLayout();
  flayout->addRow(tr("Date and Time"), dtLayout);

  this->setLayout(flayout);

  QObject::connect(now, SIGNAL(clicked()), this, SLOT(onSetTimeToNow()));
  QObject::connect(_dtEdit, SIGNAL(editingFinished()), this, SLOT(onSetTime()));
  QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(onUpdateTime()));
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
