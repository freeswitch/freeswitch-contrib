#include "fs_gui.h"
#include "ui_fs_gui.h"
#include "esl_oop.h"
#include <iostream>

Cfsgui::Cfsgui(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Cfsgui),
    serverDialog(new CserverManager)
{
    m_ui->setupUi(this);

    // Set the default status bar message
    m_ui->statusBar->addPermanentWidget(new QLabel(tr("Ready")),true);

    connect(m_ui->actionConnect, SIGNAL(triggered())
            , serverDialog, SLOT(show()));
    connect(serverDialog, SIGNAL(doConnect(QString,QString,QString)),
            this, SLOT(newConnectionFromDialog(QString,QString,QString)));

    }
Cfsgui::~Cfsgui()
{
    delete m_ui;
    delete serverDialog;
}

void Cfsgui::appendConsoleText(const QString text)
{
    m_ui->textConsole->append(text);
}

void Cfsgui::gotConnectedSlot()
{
    m_ui->statusBar->showMessage("Connected");
    appendConsoleText("Conneted!");
}
void Cfsgui::gotDisconnectedSlot()
{
    appendConsoleText("Disconnected!");
    m_ui->textConsole->append("Conneted!");
}

void Cfsgui::gotEventSlot(ESLevent * event)
{
    if (event->getBody())
    {
        QString text = event->getBody();

        if (text.endsWith("\r\n"))
            text.chop(2);
        if (text.endsWith("\n"))
            text.chop(1);

        appendConsoleText(text);
    }
    delete event;
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

    /* Connect signals from eslConnection */
    connect(eslConnection, SIGNAL(gotConnected(void)),
            this, SLOT(gotConnectedSlot(void)));
    connect(eslConnection, SIGNAL(gotDisconnected(void)),
            this, SLOT(gotDisconnectedSlot(void)));
    connect(eslConnection, SIGNAL(gotEvent(ESLevent*)),
            this, SLOT(gotEventSlot(ESLevent*)));
}
