#include <QtGui>
#include "eslconnection.h"
#include "servermanager.h"
#include "ui_servermanager.h"

ServerManager::ServerManager(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ServerManager)
{
    m_ui->setupUi(this);

    m_ui->serverConfigTabs->removeTab(1);

    readSettings();
    QObject::connect(m_ui->listServers, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
                     this, SLOT(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    QObject::connect(m_ui->btnAddServer, SIGNAL(clicked()),
                     this, SLOT(addServer()));
    QObject::connect(m_ui->btnDelServer, SIGNAL(clicked()),
                     this, SLOT(deleteServer()));
    QObject::connect(m_ui->btnEdit, SIGNAL(clicked()),
                     this, SLOT(editServer()));
    QObject::connect(m_ui->btnSave, SIGNAL(clicked()),
                     this, SLOT(saveServer()));
}

ServerManager::~ServerManager()
{
    delete m_ui;
}

ESLconnection *ServerManager::getESLconnection(QString pluginName, QString name)
{
    QTreeWidgetItem * item = m_ui->listServers->selectedItems().first();
    if (name.isNull())
    {
        name = item->data(0, ServerManager::Name).toByteArray();
    }

    if(! _ESLpool.contains(name) )
    {
        _ESLpool.insert(name, new ESLconnection(item->data(0, ServerManager::Host).toByteArray(),
                                                item->data(0, ServerManager::Port).toByteArray(),
                                                item->data(0, ServerManager::Password).toByteArray(),
                                                item->data(0, ServerManager::Name).toByteArray()));
        if ( _ESLpoolCount.contains(name) )
            qDebug() << "Something really bad happened here.";

        QList<QString> pluginRefs;
        pluginRefs.append(pluginName);
        _ESLpoolCount.insert(name, pluginRefs);
    }
    else
    {
        if ( !_ESLpoolCount.contains(name) )
            qDebug() << "Something really bad happened here.";

        QList<QString> list = _ESLpoolCount.value(name);
        list.append(pluginName);
    }
    return _ESLpool.value(name);
}

void ServerManager::endESLconnection(QString pluginName, QString name)
{
    if ( _ESLpool.contains(name) )
    {
        QList<QString> pluginList = _ESLpoolCount.value(name);
        for ( int i=0; i < pluginList.size();  i++)
        {
            if ( pluginList[i] == pluginName )
            {
                pluginList.removeAt(i);
                _ESLpoolCount.insert(name, pluginList);
                break;
            }
        }
        if ( _ESLpoolCount.value(name).count() == 0 )
        {
            qDebug() << "We are deleting";
            delete _ESLpool.take(name);
            _ESLpoolCount.remove(name);
        }
    }
}

void ServerManager::changeEvent(QEvent *e)
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

void ServerManager::currentItemChanged (QTreeWidgetItem * current, QTreeWidgetItem * /*previous*/)
{
    if (current)
    {
        m_ui->lineHost->setText(current->data(0,ServerManager::Host).toString());
        m_ui->linePassword->setText(current->data(0,ServerManager::Password).toString());
        m_ui->linePort->setText(current->data(0,ServerManager::Port).toString());
        m_ui->btnConnect->setEnabled(true);
        m_ui->btnEdit->setEnabled(true);
        m_ui->lineHost->setEnabled(false);
        m_ui->linePassword->setEnabled(false);
        m_ui->linePort->setEnabled(false);
        m_ui->btnSave->setEnabled(false);
    }
    else
    {
        m_ui->btnEdit->setEnabled(false);
        m_ui->btnConnect->setEnabled(false);
    }
}

void ServerManager::addServer()
{
    bool ok;
    QString text = QInputDialog::getText(this, tr("New Server Name"),
                                         tr("Server Name:"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty())
    {
        QSettings settings;
        settings.beginGroup("Servers");
        settings.beginGroup(text);
        settings.setValue("name", text);
        settings.setValue("port", 8021);
        settings.endGroup();
        settings.endGroup();
        readSettings();
        m_ui->listServers->setCurrentItem(m_ui->listServers->findItems(text, Qt::MatchExactly, 0).first());
    }
}

void ServerManager::editServer()
{
    m_ui->lineHost->setEnabled(true);
    m_ui->linePassword->setEnabled(true);
    m_ui->linePort->setEnabled(true);
    m_ui->btnSave->setEnabled(true);
    m_ui->btnEdit->setEnabled(false);
    m_ui->lineHost->setFocus();
}

void ServerManager::saveServer()
{
    QTreeWidgetItem * item = m_ui->listServers->selectedItems().first();
    item->setData(0, ServerManager::Host, m_ui->lineHost->text());
    item->setData(0, ServerManager::Password, m_ui->linePassword->text());
    item->setData(0, ServerManager::Port, m_ui->linePort->text());

    QSettings settings;
    settings.beginGroup("Servers");
    settings.beginGroup(item->data(0, ServerManager::Name).toString());
    settings.setValue("host", m_ui->lineHost->text());
    settings.setValue("password", m_ui->linePassword->text());
    settings.setValue("port", m_ui->linePort->text());
    settings.endGroup();
    settings.endGroup();

    m_ui->lineHost->setEnabled(false);
    m_ui->linePassword->setEnabled(false);
    m_ui->linePort->setEnabled(false);
    m_ui->btnSave->setEnabled(false);
    m_ui->btnEdit->setEnabled(true);
}

void ServerManager::deleteServer()
{
    QTreeWidgetItem *item = m_ui->listServers->selectedItems().first();
    QSettings settings;
    settings.beginGroup("Servers");
    settings.remove(item->data(0, ServerManager::Name).toString());
    delete item;
}

void ServerManager::readSettings()
{
    m_ui->listServers->clear();
    QSettings settings;
    settings.beginGroup("Servers");
    foreach (QString server, settings.childGroups())
    {
        settings.beginGroup(server);
        QTreeWidgetItem *serverItem = new QTreeWidgetItem(m_ui->listServers->invisibleRootItem());
        serverItem->setIcon(0, QIcon(":/icons/server_icon.png"));
        serverItem->setText(0, settings.value("name").toString());
        serverItem->setData(0, ServerManager::Name, settings.value("name"));
        serverItem->setData(0, ServerManager::Host, settings.value("host"));
        serverItem->setData(0, ServerManager::Port, settings.value("port"));
        serverItem->setData(0, ServerManager::Password, settings.value("password"));
        settings.endGroup();
    }
    settings.endGroup();
}

