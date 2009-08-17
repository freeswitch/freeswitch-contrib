#include <QtGui>
#include <QDir>
#include <QtPlugin>
#include "servermanager.h"
#include "settingsdialog.h"
#include "appmanager.h"
#include "interfaces.h"
#include "ui_config_plugin_widget.h"
#if defined(Q_OS_WIN)
#include "switch_version.h"
#endif

AppManager::AppManager(QObject *parent)
    : QObject(parent)
{
    reallyClose = NULL;
    QObject::connect(qApp, SIGNAL(lastWindowClosed()),
                     this, SLOT(lastWindowClosed()));

    QCoreApplication::setOrganizationName("FreeSWITCH");
    QCoreApplication::setApplicationName("FsGui");

    settingsDialog = new SettingsDialog();
    QWidget *base = new QWidget;
    pluginConfigPage = new Ui::pluginConfigPage();
    pluginConfigPage->setupUi(base);
    settingsDialog->addConfigItem(new QListWidgetItem("Plugins"), base);
    QObject::connect(pluginConfigPage->btnRun, SIGNAL(clicked()),
                     this, SLOT(runPlugin()));
    QObject::connect(pluginConfigPage->btnChangePluginDir, SIGNAL(clicked()),
                     this, SLOT(changePluginDir()));
    QObject::connect(settingsDialog, SIGNAL(accepted()),
                     this, SLOT(writeSettings()));
    QObject::connect(pluginConfigPage->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                     this, SLOT(pluginSelected(QListWidgetItem*, QListWidgetItem*)));

    serverManager = new ServerManager;

    qApp->setWindowIcon(QIcon(":/icons/fsgui_logo.png"));

    readSettings();
    createActions();
    loadPlugins();
    settingsDialog->show();
}

void AppManager::changePluginDir()
{
     QString dir = QFileDialog::getExistingDirectory(settingsDialog, tr("Plugin Location"),
                                                 pluginsDir->currentPath(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
     if(!dir.isEmpty())
     {
         pluginConfigPage->linePluginDir->setText(dir);
         pluginsDir->setCurrent(dir);
         writeSettings();
         unloadPlugins();
         loadPlugins();
     }
}

void AppManager::createActions()
{
    action_exit = new QAction(tr("E&xit"), this);
    action_exit->setShortcut(tr("Ctrl+Q"));
    action_exit->setToolTip(tr("Exit FsGui"));
    QObject::connect(action_exit, SIGNAL(triggered()),
                     qApp, SLOT(closeAllWindows()));

    action_preferences = new QAction(tr("&Preferences"), this);
    QObject::connect(action_preferences, SIGNAL(triggered()),
                     settingsDialog, SLOT(show()));

    action_about = new QAction(tr("&About"), this);
    action_about->setShortcut(QKeySequence::HelpContents);
    QObject::connect(action_about, SIGNAL(triggered()),
                     this, SLOT(about()));
}


void AppManager::createMenus(QMenuBar * menu_bar)
{
    bool file_exists = false;
    bool edit_exists = false;
    bool help_exists = false;
    QList<QAction *> list = menu_bar->actions();
    for (int i = 0; i < list.size(); ++i) {
        if (QString::compare(list.at(i)->text(), tr("&File"), Qt::CaseInsensitive) == 0)
        {
            list.at(i)->menu()->addAction(action_exit);
            file_exists = true;
        }
        if (QString::compare(list.at(i)->text(), tr("&Edit"), Qt::CaseInsensitive) == 0)
        {
            list.at(i)->menu()->addAction(action_preferences);
            edit_exists = true;
        }
        if (QString::compare(list.at(i)->text(), tr("&Help"), Qt::CaseInsensitive) == 0)
        {
            list.at(i)->menu()->addAction(action_about);
            help_exists = true;
        }
    }
    if (file_exists == false)
    {
        QMenu * menu_file = new QMenu(tr("&File"), menu_bar);
        menu_file->addAction(action_exit);
        menu_bar->insertMenu(list.at(0), menu_file);
    }
    if (edit_exists == false)
    {
        QMenu * menu_edit = new QMenu(tr("&Edit"), menu_bar);
        menu_edit->addAction(action_preferences);
        menu_bar->insertMenu(list.at(1), menu_edit);
    }
    if (help_exists == false)
    {
        QMenu *menu_help = menu_bar->addMenu(tr("&Help"));
        menu_help->addAction(action_about);
    }
}

void AppManager::readSettings()
{
    pluginsDir = new QDir;
    QSettings settings;
    QString dir = settings.value("PluginDir", FSGUI_PLUGINDIR).toString();
    pluginsDir->setCurrent(dir);
    pluginConfigPage->linePluginDir->setText(pluginsDir->absolutePath());
}

void AppManager::writeSettings()
{
    QSettings settings;
    settings.setValue("PluginDir", pluginConfigPage->linePluginDir->text());
}

void AppManager::unloadPlugins()
{
    /* Pop and unload all plugins doing proper cleanup */
    for (int i = 0; i < list_available_monitor_plugins.size(); i++)
    {
        QMapIterator<QListWidgetItem*, QWidget*> j(*list_available_monitor_plugins.at(i)->_configItems);
        while(j.hasNext())
        {
            j.next();
            settingsDialog->removeConfigItem(j.key(), j.value());
        }
        list_available_monitor_plugins.at(i)->_loader->unload();
        delete list_available_monitor_plugins.takeAt(i);
    }
}

void AppManager::loadPlugins()
{
    pluginConfigPage->listWidget->clear();

    foreach (QString fileName, pluginsDir->entryList(QDir::Files))
    {
        QPluginLoader *loader = new QPluginLoader(pluginsDir->absoluteFilePath(fileName));
        if (MonitorInterface *interface = qobject_cast<MonitorInterface *>(loader->instance()))
        {
            if(QMainWindow *interface_window = qobject_cast<QMainWindow *>(loader->instance()))
            {
                MonitorPlugin *plugin = new MonitorPlugin(loader);
                list_available_monitor_plugins.append(plugin);
                interface_window->setWindowTitle(QString("FsGui - ") + interface_window->windowTitle());
                interface_window->setAttribute(Qt::WA_QuitOnClose);
                interface->addConfigItems(settingsDialog, *plugin->_configItems);

                /* Add config items */
                QMapIterator<QListWidgetItem*, QWidget*> i(*plugin->_configItems);
                while(i.hasNext())
                {
                    i.next();
                    settingsDialog->addConfigItem(i.key(), i.value());
                }

                interface->setServerManager(serverManager);
                createMenus(interface_window->menuBar());
                pluginConfigPage->listWidget->addItem(QString("%1\n     %2").arg(interface->appName(),
                                                                             interface->appDescription()));
            }
            else
            {
                QMessageBox::critical(0, tr("Error!"),
                                      tr("Plugin %1 does not have the proper main Window"));
            }
        }
    }
    if (list_available_monitor_plugins.isEmpty())
    {
        QMessageBox::critical(0, tr("Error!"),
                                      tr("No available plugins. Install compatible plugins before running FsGui."));
    }
}

void AppManager::runPlugin()
{
    if (list_available_monitor_plugins.at(pluginConfigPage->listWidget->currentRow())->_loader->isLoaded())
    {
        MonitorInterface * interface = qobject_cast<MonitorInterface *>(list_available_monitor_plugins.at(pluginConfigPage->listWidget->currentRow())->_loader->instance());
        interface->newInstance();
        settingsDialog->hide();
    }
}

void AppManager::lastWindowClosed()
{
    if (!reallyClose)
    {
        reallyClose = new QMessageBox(settingsDialog);
        reallyClose->setText("Do you really want to close FsGui?");
        reallyClose->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        reallyClose->setDefaultButton(QMessageBox::Yes);
        reallyClose->setIcon(QMessageBox::Warning);
        int ret = reallyClose->exec();
        if(ret == QMessageBox::Yes)
        {
            for (int i = 0; i < list_available_monitor_plugins.size(); ++i)
            {
                list_available_monitor_plugins.takeAt(i)->_loader->unload();
            }
        }
        else
        {
            delete reallyClose;
            reallyClose = NULL;
            settingsDialog->show();
            settingsDialog->raise();
            settingsDialog->activateWindow();
        }
    }
}

void AppManager::pluginSelected(QListWidgetItem *current, QListWidgetItem */*previous*/)
{
    if (current)
        pluginConfigPage->btnRun->setEnabled(true);
    else
        pluginConfigPage->btnRun->setEnabled(false);
}

void AppManager::about()
{
#if defined(Q_OS_WIN)
    QMessageBox::about(qobject_cast<QMainWindow *>(list_available_monitor_plugins.first()->_loader->instance()),
                       tr("About FsGui"),
                       tr("Compiled with FreeSWITCH version: %1").arg(SWITCH_VERSION_FULL));
#else
    QMessageBox::about(qobject_cast<QMainWindow *>(list_available_monitor_plugins.first()->_loader->instance()),
                       tr("About FsGui"),
                       tr("Version: %1").arg(FSGUI_SVN_VERSION));
#endif
}
