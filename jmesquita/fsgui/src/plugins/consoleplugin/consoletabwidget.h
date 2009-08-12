#ifndef CONSOLETABWIDGET_H
#define CONSOLETABWIDGET_H

#include <QtGui/QWidget>
#include "sortfilterproxymodel.h"

namespace Ui {
    class ConsoleTabWidget;
}

class QStandardItem;
class QStandardItemModel;
class QTimer;
class ESLconnection;
class ESLevent;

class ConsoleTabWidget : public QWidget {
    Q_OBJECT
public:
    ConsoleTabWidget(QWidget *parent, ESLconnection *eslconnection);
    ~ConsoleTabWidget();

public slots:
    void clearConsoleContents();
    void flipScrollTimer();
    void setAutomaticScroll(bool enabled);
    bool getAutomaticScroll();

protected:
    void changeEvent(QEvent *e);

private slots:
    void conditionalScroll();
    void find();
    void findStringChanged(QString);
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
    QModelIndexList foundItems;
    SortFilterProxyModel *model;
    ESLconnection *esl;
    QTimer *scrollTimer;
    bool findNext;
    bool autoScroll;

};

#endif // CONSOLETABWIDGET_H
