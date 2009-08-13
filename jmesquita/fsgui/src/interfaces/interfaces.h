#ifndef INTERFACES_H
#define INTERFACES_H

#include <QtPlugin>

class QListWidgetItem;
class QWidget;
class ESLevent;
class SettingsDialog;
class ServerManager;

/*!
  \brief This interface is provided to extend FsGui with applications that
  could do any kind of monitoring of any kinda of data inside FreeSWITCH
  or its components
  */
class MonitorInterface
{
public:
    virtual ~MonitorInterface() { }
    /*!
      \brief This returns the plugin itself so we can connect signals/slots
      */
    virtual QObject* getMonitorInterfaceObject() = 0;
    /*!
      \brief This should return the name of the application
      */
    virtual QString appName() = 0;
    /*!
      \brief This should return the application description.
      */
    virtual QString appDescription() = 0;
    /*!
      \brief Add a new configuration item to the settings dialog
      \param[in] SettingsDialog A pointer to the settings dialog
      \param[out] QMap<QListWidgetItem *, QWidget *> A QMap that has a items associated to config pages.
      */
    virtual void addConfigItems(SettingsDialog *, QMap<QListWidgetItem *, QWidget *>&) = 0;
    /*!
      \brief Sets a pointer to ServerManager object
      */
    virtual void setServerManager(ServerManager *serverManager) = 0;
    /*!
      \brief Run a new instance of this application.
    */
    virtual void newInstance() = 0;
};

Q_DECLARE_INTERFACE(MonitorInterface, "com.fsgui.fsgui.MonitorInterface/1.0")
#endif // INTERFACES_H
