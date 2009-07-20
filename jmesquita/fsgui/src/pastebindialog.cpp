#include <QtGui>
#include <QtNetwork>
#include "pastebindialog.h"
#include "ui_pastebindialog.h"

pastebinDialog::pastebinDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::pastebinDialog)
{
    m_ui->setupUi(this);

    connect(this, SIGNAL(accepted()),
            this, SLOT(pasteIt()));
}

pastebinDialog::~pastebinDialog()
{
    delete m_ui;
}

void pastebinDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
void pastebinDialog::pastebinFinished(int req, bool isError)
{
    QMessageBox msgBox;
    if (isError)
    {
        msgBox.setText(QString("Error on pastebin: %1").arg(pastebinHttp->errorString()));
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.exec();
    }
    else
    {
        msgBox.setText("Pastebin correctly pasted!");
        msgBox.setIcon(QMessageBox::Information);
        msgBox.exec();
    }
}
void pastebinDialog::pasteIt()
{

    pastebinHttp = new QHttp(this);

    /* TODO: Make a progress thing? */
    connect(pastebinHttp, SIGNAL(requestFinished(int,bool)),
            this, SLOT(pastebinFinished(int,bool)));

    QHttpRequestHeader header( "POST",  "/pastebin.php") ;
    header.setValue( "Host", "pastebin.freeswitch.org" ) ;
    header.setContentType( "application/x-www-form-urlencoded" ) ;

    pastebinHttp->setHost(m_ui->lineURL->text(), QHttp::ConnectionModeHttp, m_ui->linePort->text().toInt());
    pastebinHttp->setUser(m_ui->lineUsername->text(), m_ui->linePassword->text());
    header.setValue( "User-Agent", "FsGUI");
    QString params("parent_pid=&format=fslog");
    params.append("&code2=%1").arg(text);
    if (m_ui->lineName->text().trimmed() == "")
        params.append("&poster=%1&paste=Send&remember=0").arg(tr("Anonymous"));
    if (m_ui->radioDay->isChecked())
        params.append("&expiry=%1").arg("d");
    if (m_ui->radioMonth->isChecked())
        params.append("&expiry=%1").arg("m");
    if (m_ui->radioForever->isChecked())
        params.append("&expiry=%1").arg("f");
    pastebinHttp->request(header, params.toUtf8());
}
void pastebinDialog::setText(QString consoleText)
{
    text = consoleText;
}
