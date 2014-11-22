#include "timedelegate.hh"
#include <QLineEdit>
#include <QTime>


TimeDelegate::TimeDelegate(QWidget *parent) :
  QItemDelegate(parent)
{
  // pass...
}

QWidget *
TimeDelegate::createEditor(
    QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  return new QLineEdit(parent);
}

void
TimeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
  QTime time = index.model()->data(index, Qt::EditRole).toTime();
  static_cast<QLineEdit *>(editor)->setText(time.toString());
}

void
TimeDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
  model->setData(
        index, QTime::fromString(static_cast<QLineEdit *>(editor)->text()));
}

void
TimeDelegate::updateEditorGeometry(
    QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  editor->setGeometry(option.rect);
}
