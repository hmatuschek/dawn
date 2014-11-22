#ifndef DAYOFWEEKDELEGATE_HH
#define DAYOFWEEKDELEGATE_HH

#include <QItemDelegate>

class DayOfWeekDelegate : public QItemDelegate
{
  Q_OBJECT
public:
  explicit DayOfWeekDelegate(QObject *parent = 0);

  QWidget *createEditor(
      QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

  void setEditorData(QWidget *editor, const QModelIndex &index) const;

  void setModelData(
      QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

  void updateEditorGeometry(
      QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // DAYOFWEEKDELEGATE_HH
