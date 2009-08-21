#ifndef SORTFILTERPROXYMODEL_H
#define SORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <QAbstractListModel>
#include <QVector>
#include <QList>

class QBasicTimer;
class QStandardItem;
class QScrollBar;

class ConsoleModel : public QAbstractListModel
{
    Q_OBJECT
public:
    ConsoleModel (QObject *parent = 0);
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    void appendRow ( QStandardItem* item );
    void clear();
protected:
    void timerEvent(QTimerEvent *);
private:
    QList<QStandardItem *> _listDisplayModel;
    QList<QStandardItem *> _listInsertModel;
    int batchSize;
    QBasicTimer *insertionTimer;
};

class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SortFilterProxyModel(QObject *parent = 0);
    void setLogLevelFilter(int level, bool state);

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
    QVector<bool> loglevels;
};

#endif // SORTFILTERPROXYMODEL_H
