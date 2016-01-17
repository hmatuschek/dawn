#include "logmessagetable.hh"
#include <QFont>

LogMessageTable::LogMessageTable(QObject *parent)
  : QAbstractListModel(parent), LogHandler(), _messages()
{
  // pass...
}

int
LogMessageTable::rowCount(const QModelIndex &parent) const {
  return _messages.size();
}

QVariant
LogMessageTable::data(const QModelIndex &index, int role) const {
  if (Qt::DisplayRole != role) { return QVariant(); }
  if (_messages.size() <= index.row()) { return QVariant(); }

  if (Qt::DisplayRole == role) {
    QString res;
    switch (_messages[index.row()].level()) {
      case LOG_DEBUG: res = tr("debug: "); break;
      case LOG_INFO: res = tr("info: "); break;
      case LOG_WARNING: res = tr("warning: "); break;
      case LOG_ERROR: res = tr("error: "); break;
    }
    res += QString(_messages[index.row()].str().c_str());
    return res;
  } else if (Qt::FontRole == role) {
    return QFont("Courier");
  }

  return QVariant();
}

void
LogMessageTable::handle(const LogMessage &msg) {
  beginInsertRows(QModelIndex(), 0, 0);
  _messages.prepend(msg);
  endInsertRows();
}
