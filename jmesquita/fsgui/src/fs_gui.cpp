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

Cfsgui::Cfsgui(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Cfsgui),
    serverDialog(NULL),
    prefDialog(NULL)
{

        m_ui->setupUi(this);

    // Set the default status bar message
    m_ui->statusBar->showMessage(tr("Ready"));

    connect(m_ui->actionConnect, SIGNAL(triggered()),
            this, SLOT(newConnectionFromDialog()));
    connect(m_ui->actionAbout, SIGNAL(triggered()),
            this, SLOT(showAbout()));
    connect(m_ui->tabWidget, SIGNAL(tabCloseRequested(int)),
            this, SLOT(closeTab(int)));
    connect(m_ui->actionPreferences, SIGNAL(triggered()),
            this, SLOT(showPreferences()));

    m_ui->tabWidget->clear();
    showMaximized();
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
                          "to your FreeSWITCH&copy; installation."));
}
void Cfsgui::showPreferences()
{
    if (!prefDialog)
    {
        prefDialog = new preferencesDialog();
        connect(prefDialog, SIGNAL(backgroundColorChanged(QColor)),
                this, SLOT(backgroundColorChanged(QColor)));
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
        m_ui->tabWidget->addTab(page, serverDialog->getHost());
        page->init(serverDialog->getHost());
        m_ui->tabWidget->setCurrentWidget(page);
    }
    this->activateWindow();
}
void Cfsgui::closeTab(int index)
{
    QWidget *tab = m_ui->tabWidget->widget(index);
    m_ui->tabWidget->removeTab(index);
    delete tab;
}
void Cfsgui::backgroundColorChanged(QColor color)
{
    //TODO: Get tabs and set them all!
}
