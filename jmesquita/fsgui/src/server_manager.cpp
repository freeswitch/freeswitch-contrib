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

#include "server_manager.h"
#include "ui_server_manager.h"
#include "global_defines.h"

CserverManager::CserverManager(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CserverManager)
{
    m_ui->setupUi(this);

    readSettings();

    connect(m_ui->btnSave, SIGNAL(clicked()),
            this, SLOT(addNewServer()));
    connect(m_ui->btnDelete, SIGNAL(clicked()),
            this, SLOT(deleteServer()));

    connect(m_ui->serverTreeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(serverSelected(QTreeWidgetItem*,int)));
}
CserverManager::~CserverManager()
{
    delete m_ui;
}

void CserverManager::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
QString CserverManager::getHost()
{
    return m_ui->host->text();
}
QString CserverManager::getPort()
{
    return m_ui->port->text();
}
QString CserverManager::getPass()
{
    return m_ui->pass->text();
}
void CserverManager::readSettings()
{
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);
    m_ui->serverTreeWidget->clear();
    m_ui->serverTreeWidget->setFocus();
    addChildSettings(settings, 0, "servers");
}
void CserverManager::writeSettings()
{
}
void CserverManager::addChildSettings(QSettings &settings, QTreeWidgetItem *parent, const QString &group)
{
    if(!parent)
        parent = m_ui->serverTreeWidget->invisibleRootItem();
    QTreeWidgetItem *item;



    settings.beginGroup(group);

    foreach(QString group, settings.childGroups())
    {
        item = new QTreeWidgetItem(parent);
        item->setText(0, group);
    }
    settings.endGroup();
}
void CserverManager::addNewServer()
{
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);

    settings.beginGroup("servers");
    settings.beginGroup(m_ui->host->text());
    settings.setValue("password", m_ui->pass->text());
    settings.setValue("port", m_ui->port->text());
    settings.endGroup();
    settings.endGroup();

    m_ui->serverTreeWidget->clear();
    addChildSettings(settings, 0, "servers");
}
void CserverManager::deleteServer()
{
    QTreeWidgetItem *item = m_ui->serverTreeWidget->currentItem();
    if (item)
    {
        QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);
        settings.beginGroup("servers");
        settings.remove(item->text(0));
        settings.endGroup();
        readSettings();
        m_ui->host->clear();
        m_ui->pass->clear();
        m_ui->port->clear();
    }
}
void CserverManager::serverSelected(QTreeWidgetItem *item, int col)
{
    QSettings settings(SETTINGS_ORGANIZATION, SETTINGS_APPLICATION);

    settings.beginGroup("servers");
    settings.beginGroup(item->text(col));

    m_ui->host->setText(item->text(col));
    m_ui->port->setText(settings.value("port").toString());
    m_ui->pass->setText(settings.value("password").toString());

    settings.endGroup();
    settings.endGroup();
}
