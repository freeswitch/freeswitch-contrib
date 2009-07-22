#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class findDialog;
}

class QTextEdit;

class findDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(findDialog)
public:
    explicit findDialog(QWidget *parent, QTextEdit *console);
    virtual ~findDialog();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::findDialog *m_ui;
    QTextEdit *editConsole;
    int pos;

private slots:
    void findClicked();
    void cancelClicked();
};

#endif // FINDDIALOG_H
