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
#include "consolepage.h"
#include "finddialog.h"
#include <QtNetwork>

Cfsgui::Cfsgui(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Cfsgui),
    serverDialog(NULL),
    prefDialog(NULL),
    fileDialog(NULL),
    pasteDialog(NULL),
    findDlg(NULL)
{

    m_ui->setupUi(this);
    m_ui->tabWidget->clear();

    // Set the default status bar message
    m_ui->statusBar->showMessage(tr("Ready"));

    connect(m_ui->actionServerManager, SIGNAL(triggered()),
            this, SLOT(newConnectionFromDialog()));
    connect(m_ui->actionConnect, SIGNAL(triggered()),
            this, SLOT(doConnect()));
    connect(m_ui->actionDisconnect, SIGNAL(triggered()),
            this, SLOT(doDisconnect()));
    connect(m_ui->actionSaveLog, SIGNAL(triggered()),
            this, SLOT(saveLog()));
    connect(m_ui->actionPreferences, SIGNAL(triggered()),
            this, SLOT(showPreferences()));
    connect(m_ui->actionFind, SIGNAL(triggered()),
            this, SLOT(showFind()));
    connect(m_ui->actionPastebinLog, SIGNAL(triggered()),
            this, SLOT(pastebinLog()));
    connect(m_ui->actionAbout, SIGNAL(triggered()),
            this, SLOT(showAbout()));
    connect(m_ui->actionRegisterClueCon, SIGNAL(triggered()),
            this, SLOT(registerClueCon()));

    connect(m_ui->tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeTab(int)));
    connect(m_ui->tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(changeTab(int)));

}
Cfsgui::~Cfsgui()
{
    delete m_ui;
    delete serverDialog;
}
void Cfsgui::showAbout()
{
    QMessageBox::about(this, tr("About FSGui"),
                       tr("<h2>FSGui - The console for humans</h2>"
                          "<p>Author: Jo&atilde;o Mesquita &lt;jmesquita@gmail.com>"
                          "<p>This is small application that will help you connect "
                          "to your FreeSWITCH&trade; installation."
                          "<p>The FreeSWITCH&trade; images and name are trademark of"
                          " Anthony Minessale II, primary author of FreeSWITCH&trade;."));
}
void Cfsgui::showPreferences()
{
    if (!prefDialog)
    {
        prefDialog = new preferencesDialog();
        connect(prefDialog, SIGNAL(backgroundColorChanged()),
                this, SLOT(backgroundColorChanged()));
    }
    prefDialog->show();
    prefDialog->raise();
    prefDialog->activateWindow();
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
        consolePage *page = new consolePage();
        connect(page, SIGNAL(gotConnected()),
                this, SLOT(gotConnected()));
        connect(page, SIGNAL(gotDisconnected()),
                this, SLOT(gotDisconnected()));
        m_ui->tabWidget->addTab(page, serverDialog->getHost());
        page->init(serverDialog->getHost());
        m_ui->tabWidget->setCurrentWidget(page);
        page->doConnect();
    }
    this->activateWindow();
}
void Cfsgui::closeTab(int index)
{
    QWidget *tab = m_ui->tabWidget->widget(index);
    m_ui->tabWidget->removeTab(index);
    delete tab;
}
void Cfsgui::changeTab(int index)
{
    consolePage *tab = static_cast<consolePage *>(m_ui->tabWidget->widget(index));
    if (tab)
    {
        if (tab->isConnected())
        {
            gotConnected();
        }
        else
        {
            gotDisconnected();
        }
    }
}
void Cfsgui::gotConnected()
{
    m_ui->actionConnect->setDisabled(true);
    m_ui->actionDisconnect->setEnabled(true);
    m_ui->actionSaveLog->setEnabled(true);
    m_ui->actionPastebinLog->setEnabled(true);
    m_ui->actionFind->setEnabled(true);
}
void Cfsgui::gotDisconnected()
{
    m_ui->actionConnect->setEnabled(true);
    m_ui->actionDisconnect->setDisabled(true);
    m_ui->actionSaveLog->setDisabled(true);
    m_ui->actionPastebinLog->setDisabled(true);
    m_ui->actionFind->setDisabled(true);
}
void Cfsgui::showFind()
{
    consolePage *tab = static_cast<consolePage *>(m_ui->tabWidget->widget(m_ui->tabWidget->currentIndex()));
    if (tab)
    {
        if (!findDlg)
        {
            findDlg = new findDialog(this, tab->getTextDocument());
        }
        findDlg->show();
        findDlg->raise();
        findDlg->activateWindow();
    }
}
void Cfsgui::backgroundColorChanged()
{
    for (int i = 0; i < m_ui->tabWidget->count(); i++)
    {
        consolePage *tab = static_cast<consolePage *>(m_ui->tabWidget->widget(i));
        tab->setConsoleBackground();
    }
}
void Cfsgui::saveLog()
{
    consolePage *tab = static_cast<consolePage *>(m_ui->tabWidget->widget(m_ui->tabWidget->currentIndex()));
    if (tab)
    {
        if(!fileDialog)
        {
            fileDialog = new QFileDialog(this);
        }

        fileDialog->setAcceptMode(QFileDialog::AcceptSave);

        if (fileDialog->exec())
        {
            QStringList fileNames = fileDialog->selectedFiles();
            QFile file(fileNames[0]);
            file.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text);
            file.write(tab->getConsoleText().toAscii());
            file.close();
        }
    }
}
void Cfsgui::pastebinLog()
{
    consolePage *tab = static_cast<consolePage *>(m_ui->tabWidget->widget(m_ui->tabWidget->currentIndex()));
    if (tab)
    {
        if (!pasteDialog)
        {
            pasteDialog = new pastebinDialog(this);
        }
        pasteDialog->setText(tab->getConsoleText());
        pasteDialog->show();
        pasteDialog->raise();
        pasteDialog->activateWindow();
    }
}
void Cfsgui::registerClueCon()
{
    QDesktopServices::openUrl(QUrl("http://www.cluecon.com/user/register"));
}
void Cfsgui::doDisconnect()
{
    consolePage *tab = static_cast<consolePage *>(m_ui->tabWidget->widget(m_ui->tabWidget->currentIndex()));
    if (tab)
        tab->doDisconnect();
}
void Cfsgui::doConnect()
{
    consolePage *tab = static_cast<consolePage *>(m_ui->tabWidget->widget(m_ui->tabWidget->currentIndex()));
    if (tab)
        tab->doConnect();
}
