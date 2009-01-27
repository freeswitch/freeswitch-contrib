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
    void error(const QString&);
    void connectionSuccessful(void);
    void connectSlot(const QString&, const QString&, const QString&);
    void btnSendClickedSlot();
    void lineCmdEditedSlot(const QString&);
    void messageSlot(const QString&);

signals:
    void btnSendClickedSignal(const QString&);

};

#endif // FSGUI_H
