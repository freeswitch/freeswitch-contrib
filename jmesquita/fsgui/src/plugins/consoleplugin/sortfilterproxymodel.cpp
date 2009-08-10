#include <QtGui>
#include "sortfilterproxymodel.h"

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

bool SortFilterProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex index0 = sourceModel()->index(source_row, 0, source_parent);

    return loglevels.value(sourceModel()->data(index0, Qt::UserRole).toInt()) == true;
}
