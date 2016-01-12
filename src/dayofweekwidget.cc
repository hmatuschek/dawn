#include "dayofweekwidget.hh"

DayOfWeekWidget::DayOfWeekWidget(uint8_t dow, QWidget *parent)
  : QListWidget(parent)
{
  QListWidgetItem *item = new QListWidgetItem(tr("Sunday"));
  if (dow & 0x01) { item->setCheckState(Qt::Checked); }
  this->addItem(item); dow >>= 1;
  item = new QListWidgetItem(tr("Monday"));
  if (dow & 0x01) { item->setCheckState(Qt::Checked); }
  this->addItem(item); dow >>= 1;
  item = new QListWidgetItem(tr("Tuesday"));
  if (dow & 0x01) { item->setCheckState(Qt::Checked); }
  this->addItem(item); dow >>= 1;
  item = new QListWidgetItem(tr("Wednesday"));
  if (dow & 0x01) { item->setCheckState(Qt::Checked); }
  this->addItem(item); dow >>= 1;
  item = new QListWidgetItem(tr("Thursday"));
  if (dow & 0x01) { item->setCheckState(Qt::Checked); }
  this->addItem(item); dow >>= 1;
  item = new QListWidgetItem(tr("Friday"));
  if (dow & 0x01) { item->setCheckState(Qt::Checked); }
  this->addItem(item); dow >>= 1;
  item = new QListWidgetItem(tr("Saturday"));
  if (dow & 0x01) { item->setCheckState(Qt::Checked); }
  this->addItem(item); dow >>= 1;
}

uint8_t
DayOfWeekWidget::dayOfWeek() const {
  uint8_t dow = 0;
  for (int i=0; i<7; i++) {
    if (Qt::Checked == this->item(i)->checkState()) {
      dow |= (1<<i);
    }
  }
  return dow;
}

void
DayOfWeekWidget::setDayOfWeek(uint8_t dow) {
  for (int i=0; i<7; i++) {
    if ((dow>>i) & 0x1) { this->item(i)->setCheckState(Qt::Checked); }
    else { this->item(i)->setCheckState(Qt::Unchecked); }
  }
}

