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
#include <QtGui>
#include "consolepage.h"
#include "ui_consolepage.h"
#include "esl_connection.h"
#include "esl.h"

consolePage::consolePage(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::consolePage)
{
    m_ui->setupUi(this);
    connect(m_ui->comboLogLevel, SIGNAL(currentIndexChanged(int)),
            this, SLOT(loglevelChanged(int)));
}
consolePage::~consolePage()
{
    writeSettings();
    if (eslConnection->isRunning())
    {
        eslConnection->disconnect();
        eslConnection->wait();
    }
    delete eslConnection;
    delete m_ui;
}
void consolePage::init(QString host)
{
    this->host = host;
    connect(m_ui->lineCmd, SIGNAL(textChanged(QString)),
            this, SLOT(typedCommand()));
    connect(m_ui->btnSend, SIGNAL(clicked()),
            this, SLOT(sendCommand()));

    QString settingsApplication = "FSGui";
    QString settingsOrganization = "FreeSWITCH";

    QSettings settings(settingsOrganization, settingsApplication);

    settings.beginGroup("servers");
    settings.beginGroup(host);
    QString port = settings.value("port").toString();
    QString pass = settings.value("password").toString();
    settings.endGroup();
    settings.endGroup();

    eslConnection = new ESLconnection(host.toAscii(),
                                      port.toAscii(),
                                      pass.toAscii());

    /* Connect signals from eslConnection */
    connect(eslConnection, SIGNAL(gotConnected(void)),
            this, SLOT(gotConnectedSlot(void)));
    connect(eslConnection, SIGNAL(gotDisconnected(void)),
            this, SLOT(gotDisconnectedSlot(void)));
    connect(eslConnection, SIGNAL(connectionFailed(QString)),
            this, SLOT(connectionFailedSlot(QString)));
    connect(eslConnection, SIGNAL(gotEvent(ESLevent*)),
            this, SLOT(gotEventSlot(ESLevent*)));

    eslConnection->doConnect();
}
void consolePage::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
void consolePage::appendConsoleText(const QString text)
{
    m_ui->textConsole->append(text);
}
void consolePage::getDisconnectedSlot()
{
    eslConnection->disconnect();
}
void consolePage::typedCommand()
{
    m_ui->btnSend->setDisabled(m_ui->lineCmd->text().isEmpty());
}
void consolePage::sendCommand()
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
void consolePage::gotConnectedSlot()
{
    appendConsoleText(tr("Connected!"));
    m_ui->lineCmd->setEnabled(eslConnection->connected());
    m_ui->comboLogLevel->setEnabled(eslConnection->connected());
    m_ui->lineCmd->setFocus();
    readSettings();
}
void consolePage::gotDisconnectedSlot()
{
    appendConsoleText(tr("Disconnected!"));
    m_ui->lineCmd->setEnabled(eslConnection->connected());
    m_ui->comboLogLevel->setEnabled(eslConnection->connected());
}
void consolePage::connectionFailedSlot(QString msg)
{
    appendConsoleText("Connection Failed: "+msg);
    m_ui->lineCmd->setEnabled(eslConnection->connected());
    m_ui->comboLogLevel->setEnabled(eslConnection->connected());
}
void consolePage::gotEventSlot(ESLevent * event)
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
void consolePage::loglevelChanged(int loglevel)
{
    if (!eslConnection->connected())
    {
        appendConsoleText(QString("Cannot change loglevel if not connected."));
        return;
    }
    ESLevent *event = eslConnection->sendRecv(QString("log %1").arg(loglevel).toAscii());
    if (event)
    {
        appendConsoleText(QString("Changed loglevel to %1").arg(m_ui->comboLogLevel->currentText()));
    }
    else
    {
        appendConsoleText(QString("Could not change loglevel"));
    }
}
void consolePage::readSettings()
{
    QString settingsApplication = "FSGui";
    QString settingsOrganization = "FreeSWITCH";
    QSettings settings(settingsOrganization, settingsApplication);
    m_ui->comboLogLevel->setCurrentIndex(settings.value(QString("servers/%1/loglevel").arg(host)).toInt());
}
void consolePage::writeSettings()
{
    QString settingsApplication = "FSGui";
    QString settingsOrganization = "FreeSWITCH";
    QSettings settings(settingsOrganization, settingsApplication);
    settings.setValue(QString("servers/%1/loglevel").arg(host), m_ui->comboLogLevel->currentIndex());
}
