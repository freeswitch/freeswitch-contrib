#ifndef COMPLETERS_H
#define COMPLETERS_H

#include <QCompleter>

class QStringListModel;

class cmdHistory : public QCompleter
{
    Q_OBJECT

public:
    cmdHistory();
    ~cmdHistory();
    void addStringToModel(QString);
private:
    QStringListModel *listModel;
    QAbstractItemModel *modelFromFile(const QString& fileName);
    void writeHistoryFile(const QString& fileName);
};

#endif // COMPLETERS_H
