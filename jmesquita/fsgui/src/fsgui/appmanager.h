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

    QList<QPluginLoader *> list_available_monitor_plugins; /*< List of available Monitor plugins */
    QAction *action_exit; /*< Exit the application */
    QAction *action_preferences; /*< Show the preferences dialog */
    QAction *action_about; /*< Calls the about dialog */
    QMenu *menu_file; /*< The file menu on menu bar */
    QMenu *menu_edit; /*< The edit menu on menu bar */
    QMenu *menu_help; /*< The help menu on menu bar */
    ServerManager *serverManager; /*< The dialog that can fetch new esl connections from the config */
    SettingsDialog *settings; /*< Settings dialog */
    Ui::pluginConfigPage *pluginConfigPage; /*< The plugin config page being used */
    QMessageBox *reallyClose;

private slots:
    /*!
      \brief Run the selected plugin.
      */
    void runPlugin();
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
};

#endif // APPMANAGER_H
