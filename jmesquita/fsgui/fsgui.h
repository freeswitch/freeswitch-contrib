#ifndef FSGUI_H
#define FSGUI_H

#include <QtGui/QMainWindow>
#include "conn_event_handler.h"


namespace Ui
{
    class FSGuiClass;
}

class FSGui : public QMainWindow
{
    Q_OBJECT

public:
    FSGui(QWidget *parent = 0);
    ~FSGui();

private:
    Ui::FSGuiClass *ui;
    conn_event_handler* socket;
    QDialog * serverLogin;
    QLabel * statusBarLabel;

private slots:
    void error(QString);
    void connectionSuccessful(void);
    void connectSlot(QString, QString, QString);
    void btnSendClickedSlot();
    void lineCmdEditedSlot(QString);
    void messageSlot(QString);

signals:
    void btnSendClickedSignal(QString);

};

#endif // FSGUI_H
