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
#include "completers.h"
#include "ui_consolepage.h"
#include "esl_connection.h"
#include "esl.h"
#include "global_defines.h"

consolePage::consolePage(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::consolePage),
    histCompleter(new cmdHistory),
    lineCmdEventFilter(new keyPressEventFilter(histCompleter))
{
    m_ui->setupUi(this);
    connect(m_ui->comboLogLevel, SIGNAL(currentIndexChanged(int)),
            this, SLOT(loglevelChanged(int)));
    setConsoleBackground();
    m_ui->lineCmd->installEventFilter(lineCmdEventFilter);
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
    delete histCompleter;
    delete m_ui;
}
void consolePage::setConsoleBackground()
{
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);
    m_ui->textConsole->setPalette(settings.value("consoleBackgroundColor",QColor(Qt::white)).value<QColor>());
}
int consolePage::isConnected()
{
    return eslConnection->connected();
}
void consolePage::doDisconnect()
{
    eslConnection->disconnect();
}
void consolePage::doConnect()
{
    eslConnection->doConnect();
}
void consolePage::init(QString host)
{
    this->host = host;
    connect(m_ui->lineCmd, SIGNAL(textChanged(QString)),
            this, SLOT(typedCommand()));
    connect(m_ui->btnSend, SIGNAL(clicked()),
            this, SLOT(sendCommand()));

    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);

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
    connect(eslConnection, SIGNAL(gotConsoleEvent(ESLeventLog*)),
            this, SLOT(gotConsoleEventSlot(ESLeventLog*)));

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
    ESLevent *e = new ESLevent(eslConnection->bgapi(cmdList[0].toAscii(), args.toAscii()));
    gotEventSlot(e);
    histCompleter->addStringToModel(m_ui->lineCmd->text());
    m_ui->lineCmd->clear();
}
void consolePage::gotConnectedSlot()
{
    appendConsoleText(tr("Connected!"));
    m_ui->lineCmd->setEnabled(eslConnection->connected());
    m_ui->comboLogLevel->setEnabled(eslConnection->connected());
    m_ui->lineCmd->setFocus();
    readSettings();
    eslConnection->sendRecv("event plain BACKGROUND_JOB");
    emit gotConnected();
}
void consolePage::gotDisconnectedSlot()
{
    appendConsoleText(tr("Disconnected!"));
    m_ui->lineCmd->setEnabled(eslConnection->connected());
    m_ui->comboLogLevel->setEnabled(eslConnection->connected());
    emit gotDisconnected();
}
void consolePage::connectionFailedSlot(QString msg)
{
    appendConsoleText("Connection Failed: "+msg);
    m_ui->lineCmd->setEnabled(eslConnection->connected());
    m_ui->comboLogLevel->setEnabled(eslConnection->connected());
    emit gotDisconnected();
}
void consolePage::gotEventSlot(ESLevent * event)
{
    m_ui->textConsole->setTextColor(Qt::black);
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
void consolePage::gotConsoleEventSlot(ESLeventLog * event)
{
    m_ui->textConsole->setTextColor(event->getConsoleColor());
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
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);
    m_ui->comboLogLevel->setCurrentIndex(settings.value(QString("servers/%1/loglevel").arg(host)).toInt());
}
void consolePage::writeSettings()
{
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);
    settings.setValue(QString("servers/%1/loglevel").arg(host), m_ui->comboLogLevel->currentIndex());
}
/************************************************************************/
/* keyPressEventFilter                                                  */
/************************************************************************/
keyPressEventFilter::keyPressEventFilter(cmdHistory* histCompleter) :
        histCompleter(histCompleter){}
keyPressEventFilter::~keyPressEventFilter(){}
bool keyPressEventFilter::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        // TODO: Might be good to check settings
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)
        {
            QLineEdit * lineCmd = static_cast<QLineEdit *>(obj);
            lineCmd->setCompleter(histCompleter);
            histCompleter->setCompletionPrefix(lineCmd->text());
            histCompleter->complete();
            return true;
        }
    }
    return QObject::eventFilter(obj, e);
}
