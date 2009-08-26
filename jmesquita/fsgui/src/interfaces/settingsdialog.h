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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class SettingsDialog;
}
class QListWidgetItem;
class QSettings;

/*!
  \brief This is a centralized preferences dialog. AppManager will call a method on
  each plugin interface that can then populate its own configuration pages and bind to
  signals of this dialog to control its settings.
  */
class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    /*!
      \brief Adds config a item/page pair
      \param Item to be added to config list
      \param The corresponding page
      */
    void addConfigItem(QListWidgetItem *item, QWidget *page);
    /*!
      \brief Removes a config item/page pair
      \param Item to be removed from the list
      \param The page to be removed
      */
    void removeConfigItem(QListWidgetItem *item, QWidget *page);

protected:
    /*!
      \brief Used for changing translation on runtime
      */
    void changeEvent(QEvent *e);

private slots:
    /*!
      \brief Change the configuration page
      \param The current page selected
      \param The previous page
      */
    void changePage(QListWidgetItem*, QListWidgetItem*);

private:
    Ui::SettingsDialog *m_ui; /*< Holds the UI specified on the ui file. */
    QSettings *settings; /*< Holds the settings. */
};

#endif // SETTINGSDIALOG_H
