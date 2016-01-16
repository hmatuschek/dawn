#include "alarmsettingwidget.hh"
#include <QVBoxLayout>
#include <QDialogButtonBox>

/* ******************************************************************************************** *
 * Implementation of AlarmSettingWidget
 * ******************************************************************************************** */
AlarmSettingWidget::AlarmSettingWidget(Dawn &dawn, size_t idx, QWidget *parent)
  : QLabel(parent), _dawn(dawn), _idx(idx)
{
  // Assemble style
  setAlignment(Qt::AlignCenter);
  setMargin(20);
  setTextFormat(Qt::RichText);
  setMouseTracking(true);
  setWordWrap(true);
  setStyleSheet("QLabel { font-size: 18pt; background-color: white; }"
                ":hover{ background-color: lightblue; }");
  setCursor(Qt::PointingHandCursor);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
  updateText();
}

void
AlarmSettingWidget::updateText() {
  // Get alarm setting
  Dawn::Alarm alarm = _dawn.alarm(_idx);
  QStringList days;
if (0b1111111 == alarm.dowFlags) { days.clear(); days << tr("Every day"); }
  else if (0b1000001 == alarm.dowFlags) { days.clear(); days << tr("Weekend"); }
  else if (0b0111110 == alarm.dowFlags) { days.clear(); days << tr("Work day"); }
  else {
    if (Dawn::MONDAY == (alarm.dowFlags & Dawn::MONDAY)) { days << tr("Monday"); }
    if (Dawn::TUESDAY == (alarm.dowFlags & Dawn::TUESDAY)) { days << tr("Tuesday"); }
    if (Dawn::WEDNESDAY == (alarm.dowFlags & Dawn::WEDNESDAY)) { days << tr("Wednesday"); }
    if (Dawn::THURSDAY == (alarm.dowFlags & Dawn::THURSDAY)) { days << tr("Thursday"); }
    if (Dawn::FRIDAY == (alarm.dowFlags & Dawn::FRIDAY)) { days << tr("Friday"); }
    if (Dawn::SATURDAY == (alarm.dowFlags & Dawn::SATURDAY)) { days << tr("Saturday"); }
    if (Dawn::SUNDAY == (alarm.dowFlags & Dawn::SUNDAY)) { days << tr("Sunday"); }
  }

  if (days.size()==1) {
    setText(tr("%0 at %1").arg(days.first(), alarm.time.toString("HH:mm")));
  } else if (days.size()){
    setText(tr("%0 and %1 at %2").arg(
              days.mid(0, days.size()-1).join(", "), days.last(), alarm.time.toString("HH:mm")));
  } else {
    setText(tr("Disabled"));
  }
}

void
AlarmSettingWidget::mouseReleaseEvent(QMouseEvent *ev) {
  Q_UNUSED(ev);
  AlarmSettingDialog dialog(_dawn.alarm(_idx));
  if (QDialog::Accepted != dialog.exec()) { return; }
  _dawn.setAlarm(_idx, dialog.alarm());
  updateText();
}


/* ******************************************************************************************** *
 * Implementation of AlarmSettingDialog
 * ******************************************************************************************** */
AlarmSettingDialog::AlarmSettingDialog(const Dawn::Alarm &alarm, QWidget *parent)
  : QDialog(parent)
{
  setWindowTitle(tr("Configure alarm"));
  _dow = new DayOfWeekWidget(alarm.dowFlags);
  _time = new QTimeEdit(alarm.time);

  QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Save |
                                                QDialogButtonBox::Cancel);

  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(_dow);
  layout->addWidget(_time);
  layout->addWidget(bbox);

  connect(bbox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(bbox, SIGNAL(rejected()), this, SLOT(reject()));

  setLayout(layout);
}

Dawn::Alarm
AlarmSettingDialog::alarm() const {
  Dawn::Alarm alarm;
  alarm.dowFlags = _dow->dayOfWeek();
  alarm.time = _time->time();
  return alarm;
}


/* ******************************************************************************************** *
 * Implementation of AlarmSettingsWidget
 * ******************************************************************************************** */
AlarmSettingsWidget::AlarmSettingsWidget(Dawn &dawn, QWidget *parent)
  : QWidget(parent), _dawn(dawn)
{
  QVBoxLayout *layout = new QVBoxLayout();
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(1);
  for (size_t i=0; i<dawn.numAlarms(); i++) {
    layout->addWidget(new AlarmSettingWidget(_dawn, i));
  }
  setLayout(layout);
}
