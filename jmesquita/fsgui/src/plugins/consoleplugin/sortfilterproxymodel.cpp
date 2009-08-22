#include <QtGui>
#include "sortfilterproxymodel.h"

ConsoleModel::ConsoleModel (QObject *parent)
        : QAbstractListModel(parent)
{
    QSettings settings;
    batchSize = settings.value("Console/batchSize", 200).toInt();
    insertionTimer = new QBasicTimer;
    insertionTimer->start(0, this);
}

int ConsoleModel::rowCount ( const QModelIndex & /*parent*/ ) const
{
    return _listDisplayModel.count();
}

QVariant ConsoleModel::data ( const QModelIndex & index, int role ) const
{
    if (!index.isValid())
        return QVariant();

    return _listDisplayModel.at(index.row())->data(role);
}

void ConsoleModel::clear()
{
    _listDisplayModel.clear();
}

void ConsoleModel::appendRow ( QStandardItem* item )
{
    _listInsertModel.append(item);
}

void ConsoleModel::timerEvent(QTimerEvent *e)
{

    if (e->timerId() == insertionTimer->timerId())
    {
        if (!_listInsertModel.isEmpty())
        {
            int inserted_items = 0;
            int toBeInserted = 0;
            if (_listInsertModel.size() < batchSize)
            {
                toBeInserted = _listInsertModel.size() - 1;
            } else {
                 toBeInserted = batchSize - 1;
            }
            beginInsertRows( QModelIndex(), _listDisplayModel.size(), _listDisplayModel.size() + toBeInserted );
            emit layoutAboutToBeChanged();
            while( !_listInsertModel.isEmpty() && inserted_items < batchSize)
            {
                _listDisplayModel.append(_listInsertModel.takeFirst());
                inserted_items++;
            }
            endInsertRows();
            emit layoutChanged();
        }
    }
}

SortFilterProxyModel::SortFilterProxyModel(QObject *parent)
        : QSortFilterProxyModel(parent)
{
    for(int i = 0; i < 8; i++)
        loglevels.insert(i, true);
}

void SortFilterProxyModel::setLogLevelFilter(int level, bool state)
{
    loglevels.replace(level, state);
    // Let us filter
    invalidateFilter();
}

void SortFilterProxyModel::setUUIDFilterLog(QString uuid)
{
    _uuid = uuid;
    invalidateFilter();
}

bool SortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index0 = sourceModel()->index(source_row, 0, source_parent);
    bool uuidMatch = true;

    if (!_uuid.isEmpty())
        uuidMatch = (sourceModel()->data(index0, ConsoleModel::UUIDRole).toString() == _uuid);

    return (loglevels.value(sourceModel()->data(index0, Qt::UserRole).toInt()) == true
            && sourceModel()->data(index0).toString().contains(filterRegExp())
            && uuidMatch);
}
