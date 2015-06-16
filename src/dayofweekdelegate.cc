#include "dayofweekdelegate.hh"
#include <QComboBox>
#include "dawn.hh"

DayOfWeekDelegate::DayOfWeekDelegate(QObject *parent) :
  QItemDelegate(parent)
{
  // pass...
}

QWidget *
DayOfWeekDelegate::createEditor(
    QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QComboBox *box = new QComboBox(parent);
  box->addItem(tr("Every day"), 0x7f);
  box->addItem(tr("Monday"), Dawn::MONDAY);
  box->addItem(tr("Tuesday"), Dawn::TUESDAY);
  box->addItem(tr("Wednesday"), Dawn::WEDNESDAY);
  box->addItem(tr("Thursday"), Dawn::THURSDAY);
  box->addItem(tr("Friday"), Dawn::FRIDAY);
  box->addItem(tr("Saturday"), Dawn::SATURDAY);
  box->addItem(tr("Sunday"), Dawn::SUNDAY);
  return box;
}

void
DayOfWeekDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
  int idx = index.model()->data(index, Qt::EditRole).toInt();
  QComboBox *cBox = static_cast<QComboBox*>(editor);
  cBox->setCurrentIndex(idx);
}

void
DayOfWeekDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  model->setData(index, static_cast<QComboBox*>(editor)->currentIndex(), Qt::EditRole);
}

void
DayOfWeekDelegate::updateEditorGeometry(
    QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
   editor->setGeometry(option.rect);
}
