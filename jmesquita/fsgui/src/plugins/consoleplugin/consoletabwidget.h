#ifndef CONSOLETABWIDGET_H
#define CONSOLETABWIDGET_H

#include <QtGui/QWidget>
#include "sortfilterproxymodel.h"

namespace Ui {
    class ConsoleTabWidget;
}

class QStandardItem;
class QStandardItemModel;
class ESLconnection;
class ESLevent;

class ConsoleTabWidget : public QWidget {
    Q_OBJECT
public:
    ConsoleTabWidget(QWidget *parent, ESLconnection *eslconnection);
    ~ConsoleTabWidget();
    void find();

public slots:
    void clearConsoleContents();

protected:
    void changeEvent(QEvent *e);

private slots:
    void connected();
    void disconnected();
    void connectionFailed(QString);
    void gotEvent(ESLevent *);
    void addNewConsoleItem(QStandardItem *item);
    void cmdSendClicked();
    void lineCmdChanged(QString);
    void checkEmerg(bool);
    void checkAlert(bool);
    void checkCrit(bool);
    void checkError(bool);
    void checkNotice(bool);
    void checkInfo(bool);
    void checkDebug(bool);

private:
    Ui::ConsoleTabWidget *m_ui;
    QStandardItemModel *sourceModel;
    SortFilterProxyModel *model;
    ESLconnection *esl;
};

#endif // CONSOLETABWIDGET_H
