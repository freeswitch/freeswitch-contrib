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
