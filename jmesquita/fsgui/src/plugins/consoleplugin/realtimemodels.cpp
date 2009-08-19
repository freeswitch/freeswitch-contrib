#include <QtGui>
#include "realtimemodels.h"

ChannelSortModel::ChannelSortModel(QObject *parent) :
        QSortFilterProxyModel(parent)
{}

void ChannelSortModel::setUUIDFilter(QList<QString> uuid_list)
{
    _uuid_list = uuid_list;
    invalidateFilter();
}

bool ChannelSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

    if (_uuid_list.isEmpty())
        return true;

    return (_uuid_list.contains(sourceModel()->data(index0, Qt::UserRole).toString()));
}

EventSortModel::EventSortModel(QObject *parent) :
        QSortFilterProxyModel(parent)
{}

void EventSortModel::setUUIDFilter(QString uuid)
{
    _uuid = uuid;
    invalidateFilter();
}

bool EventSortModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index0 = sourceModel()->index(sourceRow, 0, sourceParent);

    if (_uuid.isEmpty())
        return false;

    return (sourceModel()->data(index0, Qt::UserRole).toString() == _uuid);
}
