#ifndef REALTIMEMODELS_H
#define REALTIMEMODELS_H

#include <QSortFilterProxyModel>

class ChannelSortModel : public QSortFilterProxyModel {

    Q_OBJECT

public:
    ChannelSortModel(QObject *parent = 0);
    void setUUIDFilter(QList<QString> uuid);

protected:
     bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    QList<QString> _uuid_list;
};

#endif // REALTIMEMODELS_H
