/*
 * Copyright (c) 2007, Anthony Minessale II
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Contributor(s):
 *
 * Joao Mesquita <jmesquita (at) gmail.com>
 *
 */

#include "fs_gui.h"
#include "ui_fs_gui.h"
#include "esl_connection.h"
#include "esl.h"

Cfsgui::Cfsgui(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Cfsgui),
    serverDialog(NULL)
{
    m_ui->setupUi(this);

    // Set the default status bar message
    m_ui->statusBar->showMessage(tr("Ready"));

    connect(m_ui->actionConnect, SIGNAL(triggered()),
            this, SLOT(newConnectionFromDialog()));
    connect(m_ui->actionAbout, SIGNAL(triggered()),
            this, SLOT(showAbout()));
    connect(m_ui->lineCmd, SIGNAL(textChanged(QString)),
            this, SLOT(typedCommand()));
    connect(m_ui->btnSend, SIGNAL(clicked()),
            this, SLOT(sendCommand()));

    /* Why QtDesigner does not allow me to delete this? */
    m_ui->tabWidget->removeTab(1);
    delete m_ui->tab_2;
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
void Cfsgui::showAbout()
{
    QMessageBox::about(this, tr("About FSGui"),
                       tr("<h2>FSGui - The console for humans</h2>"
                          "<p>Author: Jo&atilde;o Mesquita &lt;jmesquita@gmail.com>"
                          "<p>This is small application that will help you connect "
                          "to your FreeSWITCH&copy; installation."));
}
void Cfsgui::getDisconnectedSlot()
{
    eslConnection->disconnect();
}
void Cfsgui::typedCommand()
{
    m_ui->btnSend->setDisabled(m_ui->lineCmd->text().isEmpty());
}
void Cfsgui::sendCommand()
{
    QStringList cmdList = m_ui->lineCmd->text().split(" ");
    QString args("");
    for(int i = 1; i < cmdList.size(); i++)
    {
        args += cmdList[i];
        if (i != cmdList.size()-1)
            args += " ";
    }
    ESLevent *e = new ESLevent(eslConnection->api(cmdList[0].toAscii(), args.toAscii()));
    gotEventSlot(e);
    m_ui->lineCmd->clear();
}
void Cfsgui::gotConnectedSlot()
{
    m_ui->statusBar->showMessage(tr("Connected"));
    appendConsoleText(tr("Connected!"));
    m_ui->actionConnect->setDisabled(true);
    m_ui->actionDisconnect->setEnabled(true);
    m_ui->lineCmd->setEnabled(true);
    m_ui->lineCmd->setFocus();
}
void Cfsgui::gotDisconnectedSlot()
{
    m_ui->statusBar->showMessage(tr("Disconnected"));
    appendConsoleText(tr("Disconnected!"));
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setDisabled(true);
    m_ui->lineCmd->setDisabled(true);
    delete eslConnection;
}
void Cfsgui::connectionFailedSlot(QString msg)
{
    qDebug() << msg;
    m_ui->statusBar->showMessage("Connection Failed: "+msg);
    appendConsoleText("Connection Failed: "+msg);
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setDisabled(true);
    m_ui->lineCmd->setDisabled(true);
    delete eslConnection;
}
void Cfsgui::gotEventSlot(ESLevent * event)
{
    QString type (event->getHeader("Content-Type"));
    if (QString::compare("log/data", type, Qt::CaseInsensitive) == 0)
    {
        switch (atoi(event->getHeader("log-level")))
        {
        case ESL_LOG_LEVEL_NOTICE:
            {
                m_ui->textConsole->setTextColor(Qt::cyan);
                break;
            }
        case ESL_LOG_LEVEL_WARNING:
            {
                m_ui->textConsole->setTextColor(Qt::yellow);
                break;
            }
        case ESL_LOG_LEVEL_ERROR:
        case ESL_LOG_LEVEL_CRIT:
            {
                m_ui->textConsole->setTextColor(Qt::red);
                break;
            }
        case ESL_LOG_LEVEL_ALERT:
        case ESL_LOG_LEVEL_EMERG:
        case ESL_LOG_LEVEL_INFO:
            {
                m_ui->textConsole->setTextColor(Qt::green);
                break;
            }
        case ESL_LOG_LEVEL_DEBUG:
            {
                m_ui->textConsole->setTextColor(Qt::magenta);
                break;
            }
        default:
            {
                m_ui->textConsole->setTextColor(Qt::black);
            }
        }
    }
    else
    {
        m_ui->textConsole->setTextColor(Qt::black);
    }

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
    e->accept();
}
void Cfsgui::newConnectionFromDialog()
{
    if (!serverDialog)
    {
        serverDialog = new CserverManager();
    }
    serverDialog->show();
    serverDialog->raise();
    serverDialog->activateWindow();

    if (serverDialog->exec())
    {
        m_ui->statusBar->showMessage("Connecting...");
        eslConnection = new ESLconnection(serverDialog->getHost().toAscii(),
                                          serverDialog->getPort().toAscii(),
                                          serverDialog->getPass().toAscii());
        /* Connect signals from eslConnection */
        connect(eslConnection, SIGNAL(gotConnected(void)),
                this, SLOT(gotConnectedSlot(void)));
        connect(eslConnection, SIGNAL(gotDisconnected(void)),
                this, SLOT(gotDisconnectedSlot(void)));
        connect(eslConnection, SIGNAL(connectionFailed(QString)),
                this, SLOT(connectionFailedSlot(QString)));
        connect(eslConnection, SIGNAL(gotEvent(ESLevent*)),
                this, SLOT(gotEventSlot(ESLevent*)));

        /* Connect the disconnect menu */
        connect(m_ui->actionDisconnect, SIGNAL(triggered()),
                this, SLOT(getDisconnectedSlot()));

        eslConnection->doConnect();
    }
}
