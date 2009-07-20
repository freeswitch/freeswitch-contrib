#ifndef PASTEBINDIALOG_H
#define PASTEBINDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class pastebinDialog;
}

class QHttp;
class QString;

class pastebinDialog : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(pastebinDialog)
public:
    explicit pastebinDialog(QWidget *parent = 0);
    virtual ~pastebinDialog();
    void setText(QString);

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::pastebinDialog *m_ui;
    QString text;
    QHttp * pastebinHttp;

private slots:
    void pastebinFinished(int, bool);
    void pasteIt();
};

#endif // PASTEBINDIALOG_H
