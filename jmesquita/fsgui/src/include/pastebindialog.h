#ifndef PASTEBINDIALOG_H
#define PASTEBINDIALOG_H

#include <QtGui/QDialog>

namespace Ui {
    class pastebinDialog;
}

class QHttp;
class QString;
class QProgressDialog;
class QHttpResponseHeader;

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
    QString pasteURL;
    QHttp * pastebinHttp;
    int postId;
    QProgressDialog *progressDialog;

private slots:
    void pastebinFinished(int, bool);
    void pasteIt();
    void readResponseHeader(const QHttpResponseHeader &);
    void updateDataSendProgress(int, int);
};

#endif // PASTEBINDIALOG_H
