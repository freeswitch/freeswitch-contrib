#include <QtGui>
#include "completers.h"

cmdHistory::cmdHistory()
{
    this->setModel(modelFromFile(":/resources/history.txt"));
}
cmdHistory::~cmdHistory()
{
    writeHistoryFile(":/resources/history.txt");
}
void cmdHistory::addStringToModel(QString cmd)
{
    QStringList list = listModel->stringList();
    if (!list.contains(cmd, Qt::CaseInsensitive))
        list << cmd;

    listModel->setStringList(list);
    qDebug() << list;
}
QAbstractItemModel *cmdHistory::modelFromFile(const QString& fileName)
 {
     QFile file(fileName);
     if (!file.open(QFile::ReadOnly))
     {
         listModel = new QStringListModel(this);
         return listModel;
     }

     QStringList words;

     while (!file.atEnd()) {
         QByteArray line = file.readLine();
         if (!line.isEmpty())
             words << line.trimmed();
     }
     listModel = new QStringListModel(words, this);
     return listModel;
 }
void cmdHistory::writeHistoryFile(const QString& fileName)
{
    /*QFile file(fileName);
    file.remove(fileName);
    file.open(QFile::WriteOnly);
    QStringList list = listModel->stringList();
    for (int i = 0; i < list.size(); i++)
    {
        file.write(list[i].toAscii());
        qDebug() << list[i];
    }*/
}
