#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QtCore>

namespace Ui {
    class pluginConfigPage;
}
class QMainWindow;
class QAction;
class QMenu;
class QMenuBar;
class QListWidgetItem;
class QMessageBox;
class MonitorInterface;
class SettingsDialog;
class ServerManager;

/*!
  \brief Represents a loaded monitor plugin
  */
class MonitorPlugin
{
public:
    MonitorPlugin(QPluginLoader *loader) : _loader(loader), _configItems(new QMap<QListWidgetItem*, QWidget*>){}
    QPluginLoader *_loader;
    QMap<QListWidgetItem*, QWidget*> *_configItems;
};

/*!
  \brief This class will be the center of each installation
  and will load some plugins.
  */
class AppManager : public QObject
{
    Q_OBJECT

public:
    AppManager(QObject *parent = 0);

private:
    /*!
      \brief Create all actions that belong to this specific window.
      */
    void createActions();
    /*!
      \brief Create menus and override offensing default menus.
      */
    void createMenus(QMenuBar *menu_bar);
    /*!
      \brief Load all plugins populating each corresponding list
      */
    void loadPlugins();
    /*!
      \brief Unload all plugins cleaning up what is needed
      */
    void unloadPlugins();

    QList<MonitorPlugin *> list_available_monitor_plugins; /*< List of available Monitor plugins */
    QAction *action_exit; /*< Exit the application */
    QAction *action_preferences; /*< Show the preferences dialog */
    QAction *action_about; /*< Calls the about dialog */
    QMenu *menu_file; /*< The file menu on menu bar */
    QMenu *menu_edit; /*< The edit menu on menu bar */
    QMenu *menu_help; /*< The help menu on menu bar */
    ServerManager *serverManager; /*< The dialog that can fetch new esl connections from the config */
    SettingsDialog *settingsDialog; /*< Settings dialog */
    Ui::pluginConfigPage *pluginConfigPage; /*< The plugin config page being used */
    QMessageBox *reallyClose; /*< Dialog that will confirm closing the application */
    QDir *pluginsDir; /*< Directory from where we search the plugins */

private slots:
    /*!
      \brief Run the selected plugin.
      */
    void runPlugin();
    /*!
      \brief Change the plugin search directory
      */
    void changePluginDir();
    /*!
      \brief Cleanup and give plugins a chance to clean for themselves.
      */
    void lastWindowClosed();
    /*!
      \brief Just enables or disables the run button when no plugins is selected.
      */
    void pluginSelected(QListWidgetItem*, QListWidgetItem*);
    /*!
      \brief Show the about dialog.
      */
    void about();
    /*!
      \brief Write settings when convenient
      */
    void writeSettings();
    /*!
      \brief Read settings for the General section
      */
    void readSettings();

};

#endif // APPMANAGER_H
