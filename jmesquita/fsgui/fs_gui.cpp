#include "fs_gui.h"
#include "ui_fs_gui.h"
#include <iostream>

Cfsgui::Cfsgui(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Cfsgui),
    serverDialog(new CserverManager)
{
    m_ui->setupUi(this);
    connect(m_ui->actionConnect, SIGNAL(triggered())
            , serverDialog, SLOT(show()));
    connect(serverDialog, SIGNAL(doConnect(QString,QString,QString)),
            this, SLOT(newConnectionFromDialog(QString,QString,QString)));

    /*connect(eslConnection, SIGNAL(gotConnected(void)),
            this, SLOT(gotConnectedSlot(void)));
    connect(eslConnection, SIGNAL(gotDisconnected(void)),
            this, SLOT(gotDisconnectedSlot(void)));*/
}
Cfsgui::~Cfsgui()
{
    delete m_ui;
    delete serverDialog;
}

void Cfsgui::gotConnectedSlot()
{
    m_ui->textConsole->append("Conneted!");
    m_ui->statusBar->showMessage("Connected");
}
void Cfsgui::gotDisconnectedSlot()
{
    m_ui->statusBar->showMessage("Disconnected");
    m_ui->textConsole->append("Conneted!");
}
void Cfsgui::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Cfsgui::closeEvent(QCloseEvent *e)
{
    /* TODO: We have to stop threads and do cleanup */
    delete eslConnection;
    e->accept();
}

void Cfsgui::newConnectionFromDialog(QString host, QString pass, QString port)
{
    m_ui->statusBar->showMessage("Connecting...");
    eslConnection = new eslConnectionManager(host, pass, port);
}
