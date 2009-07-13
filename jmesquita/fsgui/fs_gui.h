#ifndef FS_GUI_H
#define FS_GUI_H

#include <QtGui>
#include "server_manager.h"
#include "esl_connection.h"

namespace Ui {
    class Cfsgui;
}

class Cfsgui : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY(Cfsgui)
public:
    explicit Cfsgui (QWidget *parent = 0);
    virtual ~Cfsgui();

protected:
    virtual void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);

private slots:
    void newConnectionFromDialog(QString,QString,QString);
    void gotConnectedSlot();
    void gotDisconnectedSlot();

private:
    Ui::Cfsgui *m_ui;
    CserverManager *serverDialog;
    eslConnectionManager *eslConnection;
};

#endif // FS_GUI_H
