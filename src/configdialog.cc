#include "configdialog.hh"
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QListView>


ConfigDialog::ConfigDialog(Dawn &dawn, QWidget *parent) :
  QDialog(parent), _dawn(dawn), _coreAvg(0), _ambAvg(0)
{
  setWindowTitle(tr("Settings"));

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
  //flayout->addRow(tr("CPU temperature"), _coreTemp);
  flayout->addRow(tr("Device temperature"), _ambTemp);

  QListView *msg = new QListView();
  msg->setModel(_dawn.logMessages());
  msg->setAlternatingRowColors(true);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addLayout(flayout);
  layout->addWidget(msg);
  this->setLayout(layout);

  QObject::connect(now, SIGNAL(clicked()), this, SLOT(onSetTimeToNow()));
  //QObject::connect(_dtEdit, SIGNAL(editingFinished()), this, SLOT(onSetTime()));
  QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(onUpdateTime()));

  _timer.setInterval(1000);
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
    // Update average
    if (0 == _coreAvg) {
      _coreAvg = coreTemp;
      _ambAvg = ambTemp;
    } else {
      _coreAvg = (1-1./60)*_coreAvg+coreTemp/60;
      _ambAvg = (1-1./60)*_ambAvg+ambTemp/60;
    }
    // update label
    _coreTemp->setText(tr("%0 °C (%1 °C)").arg(coreTemp, 4, 'f', 1).arg(_coreAvg, 4, 'f', 1));
    _ambTemp->setText(tr("%0 °C (%1 °C)").arg(ambTemp, 4, 'f', 1).arg(_ambAvg, 4, 'f', 1));
  }
}
