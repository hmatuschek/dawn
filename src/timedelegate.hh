#ifndef TIMEDELEGATE_HH
#define TIMEDELEGATE_HH

#include <QItemDelegate>


class TimeDelegate : public QItemDelegate
{
  Q_OBJECT

public:
  explicit TimeDelegate(QWidget *parent = 0);

  QWidget *createEditor(
      QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  void setEditorData(QWidget *editor, const QModelIndex &index) const;

  void setModelData(
      QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

  void updateEditorGeometry(
      QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // TIMEDELEGATE_HH
