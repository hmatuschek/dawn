#ifndef DAYOFWEEKDELEGATE_HH
#define DAYOFWEEKDELEGATE_HH

#include <QItemDelegate>
#include "dayofweekwidget.hh"

/** A QListView Delegate to display a drop-down selection for the day-of-week selection. */
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
