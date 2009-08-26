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
 * Joao Mesquita <jmesquita (at) freeswitch.org>
 *
 */
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
    QPluginLoader *_loader; /*< The QPluginLoader object of this plugin */
    QMap<QListWidgetItem*, QWidget*> *_configItems; /*< Holds pairs of items for the settings dialog */
};

/*!
  \brief This class is responsible for loading plugins and connecting the dots of the application.
  main() instantiates this and this class initiates the rest of the app.
  */
class AppManager : public QObject
{
    Q_OBJECT

public:
    AppManager(QObject *parent = 0);
    ~AppManager();

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
