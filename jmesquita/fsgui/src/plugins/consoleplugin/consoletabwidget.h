#ifndef CONSOLETABWIDGET_H
#define CONSOLETABWIDGET_H

#include <QtGui/QWidget>
#include "sortfilterproxymodel.h"
#include "eslevent.h"

namespace Ui {
    class ConsoleTabWidget;
}

class QStandardItem;
class QStandardItemModel;
class QTimer;
class ESLconnection;
class MonitorStateMachine;
class RealtimeStatisticsDialog;

class ConsoleTabWidget : public QWidget {
    Q_OBJECT
public:
    ConsoleTabWidget(QWidget *parent, ESLconnection *eslconnection);
    ~ConsoleTabWidget();

public slots:
    void clearConsoleContents();
    void showRealtimeStats();
    void flipScrollTimer();
    void setAutomaticScroll(bool enabled);
    bool getAutomaticScroll();

protected:
    void changeEvent(QEvent *e);

private slots:
    void conditionalScroll();
    void filterClear();
    void filterStringChanged();
    void connected();
    void disconnected();
    void connectionFailed(QString);
    void gotEvent(ESLevent);
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
    MonitorStateMachine *msm;
    RealtimeStatisticsDialog *_rtStatsDlg;
    bool findNext;
    bool autoScroll;

};

#endif // CONSOLETABWIDGET_H
