#ifndef LOGMESSAGETABLE_HH
#define LOGMESSAGETABLE_HH

#include <QAbstractListModel>
#include "logger.hh"

class LogMessageTable : public QAbstractListModel, public LogHandler
{
  Q_OBJECT

public:
  LogMessageTable(QObject *parent=0);

  int rowCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;

  void handle(const LogMessage &msg);

protected:
  QVector<LogMessage> _messages;
};

#endif // LOGMESSAGETABLE_HH
