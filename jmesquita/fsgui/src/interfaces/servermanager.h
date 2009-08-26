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
#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QtGui/QDialog>

namespace Ui {
    class ServerManager;
}

class ESLconnection;
class QTreeWidgetItem;

/*!
  \brief This dialog controls all server entries and can be used by any plugin.
  */
class ServerManager : public QDialog {
    Q_OBJECT
public:
    ServerManager(QWidget *parent = 0);
    ~ServerManager();
    /*!
      \brief Gets a new ESLconnection object from the selected server entry
      \return The ESLconnection object
      */
    ESLconnection * getESLconnection();

protected:
    /*!
      \brief Used for changing translation on runtime
      */
    void changeEvent(QEvent *e);

private slots:
    /*!
      \brief Change the line edits according to what server entry is selected
      \param current The item that was selected
      \param previous The previous item selected
      */
    void currentItemChanged ( QTreeWidgetItem * current, QTreeWidgetItem * previous );
    /*!
      \brief Used when user click on add a new server
      */
    void addServer();
    /*!
      \brief Used when user click on edit a server
      */
    void editServer();
    /*!
      \brief Used when user is saving server settings
      */
    void saveServer();
    /*!
      \brief Used when user deletes a server
      */
    void deleteServer();

private:
    Ui::ServerManager *m_ui; /*< Holds the ui for this dialog */

    /*! The roles used for each item entry on the server list */
    enum {
        Host = Qt::UserRole,
        Port,
        Password,
        Name
    };

    /*!
      \brief Read all servers and populate the interface
    */
    void readSettings();
};

#endif // SERVERMANAGER_H
