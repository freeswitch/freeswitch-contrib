#include <QtGui>
#include <QDir>
#include <QtPlugin>
#include "servermanager.h"
#include "settingsdialog.h"
#include "appmanager.h"
#include "interfaces.h"
#include "ui_config_plugin_widget.h"

AppManager::AppManager(QObject *parent)
    : QObject(parent)
{
    QObject::connect(qApp, SIGNAL(lastWindowClosed()),
                     this, SLOT(lastWindowClosed()));

    QCoreApplication::setOrganizationName("FreeSWITCH");
    QCoreApplication::setApplicationName("FsGui");
    settings = new SettingsDialog();
    QWidget *base = new QWidget;
    pluginConfigPage = new Ui::pluginConfigPage();
    pluginConfigPage->setupUi(base);
    settings->addConfigItem("Plugins", base);
    QObject::connect(pluginConfigPage->btnRun, SIGNAL(clicked()),
                     this, SLOT(runPlugin()));
    QObject::connect(pluginConfigPage->listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                     this, SLOT(pluginSelected(QListWidgetItem*, QListWidgetItem*)));

    serverManager = new ServerManager;

    qApp->setWindowIcon(QIcon(":/icons/fsgui_logo.png"));

    createActions();
    loadPlugins();
    settings->show();
}


void AppManager::createActions()
{
    action_exit = new QAction(tr("E&xit"), this);
    action_exit->setShortcut(tr("Ctrl+Q"));
    action_exit->setToolTip(tr("Exit FsGui"));
    QWidget::connect(action_exit, SIGNAL(triggered()),
                     qApp, SLOT(closeAllWindows()));

    action_preferences = new QAction(tr("&Preferences"), this);
    QWidget::connect(action_preferences, SIGNAL(triggered()),
                     settings, SLOT(show()));

    action_about = new QAction(tr("&About"), this);
    action_about->setShortcut(QKeySequence::HelpContents);
    QWidget::connect(action_about, SIGNAL(triggered()),
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
        if (QString::compare(list.at(i)->text(), tr("&Preferences"), Qt::CaseInsensitive) == 0)
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

void AppManager::loadPlugins()
{
    QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_WIN)
    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
        pluginsDir.cdUp();
        pluginsDir.cdUp();
    }
#endif
    pluginsDir.cd("plugins");
    foreach (QString fileName, pluginsDir.entryList(QDir::Files))
    {
        QPluginLoader *loader = new QPluginLoader(pluginsDir.absoluteFilePath(fileName));
        if (MonitorInterface *interface = qobject_cast<MonitorInterface *>(loader->instance()))
        {
            if(QMainWindow *interface_window = qobject_cast<QMainWindow *>(loader->instance()))
            {
                list_available_monitor_plugins.append(loader);
                interface_window->setWindowTitle(QString("FsGui - ") + interface_window->windowTitle());
                interface_window->setAttribute(Qt::WA_QuitOnClose);
                interface->addConfigItems(settings);
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
        exit(1);
    }
}

void AppManager::runPlugin()
{
    if (list_available_monitor_plugins.at(pluginConfigPage->listWidget->currentRow())->isLoaded())
    {
        MonitorInterface * interface = qobject_cast<MonitorInterface *>(list_available_monitor_plugins.at(pluginConfigPage->listWidget->currentRow())->instance());
        interface->newInstance();
        settings->hide();
    }
}

void AppManager::lastWindowClosed()
{
    for (int i = 0; i < list_available_monitor_plugins.size(); ++i)
    {
        list_available_monitor_plugins.at(i)->unload();
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
    QMessageBox::about(qobject_cast<QMainWindow *>(list_available_monitor_plugins.first()),
                       tr("About FsGui"),
                       tr("Hey, this is FsGui!!!"));
}
