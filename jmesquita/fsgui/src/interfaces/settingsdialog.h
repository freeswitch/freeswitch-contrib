#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class SettingsDialog;
}
class QListWidgetItem;
class QSettings;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();
    void addConfigItem(QListWidgetItem *item, QWidget *page);
    void removeConfigItem(QListWidgetItem *item, QWidget *page);

protected:
    void changeEvent(QEvent *e);

private slots:
    void changePage(QListWidgetItem*, QListWidgetItem*);

private:
    Ui::SettingsDialog *m_ui;
    QSettings *settings;
};

#endif // SETTINGSDIALOG_H
