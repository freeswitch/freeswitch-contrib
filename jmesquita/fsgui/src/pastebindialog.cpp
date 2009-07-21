#include <QtGui>
#include <QtNetwork>
#include "pastebindialog.h"
#include "ui_pastebindialog.h"

pastebinDialog::pastebinDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::pastebinDialog),
    progressDialog(new QProgressDialog(this))
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
void pastebinDialog::pasteIt()
{

    pastebinHttp = new QHttp(this);

    /* TODO: Make a progress thing? */
    connect(pastebinHttp, SIGNAL(requestFinished(int,bool)),
            this, SLOT(pastebinFinished(int,bool)));
     connect(pastebinHttp, SIGNAL(dataSendProgress(int, int)),
             this, SLOT(updateDataSendProgress(int, int)));
     connect(pastebinHttp, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
             this, SLOT(readResponseHeader(const QHttpResponseHeader &)));

    QHttpRequestHeader header( "POST",  "/pastebin.php") ;
    header.setValue( "Host", "pastebin.freeswitch.org" ) ;
    header.setContentType( "application/x-www-form-urlencoded" ) ;

    pastebinHttp->setHost(m_ui->lineURL->text(), QHttp::ConnectionModeHttp, m_ui->linePort->text().toInt());
    pastebinHttp->setUser(m_ui->lineUsername->text(), m_ui->linePassword->text());
    header.setValue( "User-Agent", "FsGUI");
    QString params = QString("parent_pid=&format=fslog&code2=%1").arg(QString(QUrl::toPercentEncoding(text)));

    /* Set the paste user identification */
    if (m_ui->lineName->text().trimmed().isEmpty())
    {
        params += QString("&poster=%1&paste=Send&remember=0").arg(QString(QUrl::toPercentEncoding(tr("Anonymous"))));
    }
    else
    {
        params += QString("&poster=%1&paste=Send&remember=0").arg(QString(QUrl::toPercentEncoding(m_ui->lineName->text())));
    }

    /* Set the duration of the paste */
    if (m_ui->radioDay->isChecked())
    {
        params += QString("&expiry=%1").arg("d");
    }
    if (m_ui->radioMonth->isChecked())
    {
        params += QString("&expiry=%1").arg("m");
    }
    if (m_ui->radioForever->isChecked())
    {
        params += QString("&expiry=%1").arg("f");
    }
    /* Paste it! */
    postId = pastebinHttp->request(header, params.toAscii());
    progressDialog->setWindowTitle(tr("Pastebin"));
    progressDialog->setLabelText(tr("Pasting..."));
}
void pastebinDialog::setText(QString consoleText)
{
    text = consoleText;
}
void pastebinDialog::pastebinFinished(int req, bool isError)
{
    if (req == postId)
    {
        progressDialog->hide();
        QMessageBox::information(this, tr("Sucess"),
                                 tr("You pastebin was successful"
                                " and copied to clipboard: %1.")
                                 .arg(pasteURL));
        QApplication::clipboard()->setText(pasteURL);
    }

}
void pastebinDialog::updateDataSendProgress(int bytesRead, int totalBytes)
{
    progressDialog->setMaximum(totalBytes);
    progressDialog->setValue(bytesRead);
}
void pastebinDialog::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
    switch (responseHeader.statusCode())
    {
    case 200:                   // Ok
    case 301:                   // Moved Permanently
    case 303:                   // See Other
    case 307:                   // Temporary Redirect
        // these are not error conditions
        break;

    case 302:                   // Found
        pasteURL = responseHeader.value("Location");
        break;

    default:
        QMessageBox::information(this, tr("HTTP"),
                                 tr("Pastebin Failed: %1.")
                                 .arg(responseHeader.reasonPhrase()));
        progressDialog->hide();
        pastebinHttp->abort();
    }
}
