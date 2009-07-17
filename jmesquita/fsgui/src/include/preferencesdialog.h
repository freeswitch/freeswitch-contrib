#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QtGui>
#include "global_defines.h"

namespace Ui {
    class preferencesDialog;
}

class preferencesDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(preferencesDialog)
public:
    explicit preferencesDialog(QWidget *parent = 0);
    virtual ~preferencesDialog();

protected:
    virtual void changeEvent(QEvent *e);
private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void changeBackgroundColor();
    void changeConsoleLogLevelColor();
    void changeCriticalLogLevelColor();
    void changeDebugLogLevelColor();
    void changeErrorLogLevelColor();
    void changeInfoLogLevelColor();
    void changeNoticeLogLevelColor();
    void changeWarningLogLevelColor();
    void saveSettings();
    void readSettings();
signals:
    void backgroundColorChanged();
private:
    Ui::preferencesDialog *m_ui;
    QColorDialog *colorChooser;
};

#endif // PREFERENCESDIALOG_H
