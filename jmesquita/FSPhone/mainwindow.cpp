#include <QInputDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&g_FSHost, SIGNAL(ready()),this, SLOT(fshostReady()));
    connect(&g_FSHost, SIGNAL(ringing(QString)), this, SLOT(ringing(QString)));
    connect(&g_FSHost, SIGNAL(answered(QString)), this, SLOT(answered(QString)));
    connect(&g_FSHost, SIGNAL(hungup(QString)), this, SLOT(hungup(QString)));
    connect(&g_FSHost, SIGNAL(newOutgoingCall(QString)), this, SLOT(newOutgoingCall(QString)));

    connect(ui->newCallBtn, SIGNAL(clicked()), this, SLOT(makeCall()));
    connect(ui->answerBtn, SIGNAL(clicked()), this, SLOT(paAnswer()));
    connect(ui->hangupBtn, SIGNAL(clicked()), this, SLOT(paHangup()));
    connect(ui->listCalls, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(callListDoubleClick(QListWidgetItem*)));

    g_FSHost.start();
}

MainWindow::~MainWindow()
{
    delete ui;
    QString res;
    g_FSHost.sendCmd("fsctl", "shutdown", &res);
    g_FSHost.wait();
}

void MainWindow::callListDoubleClick(QListWidgetItem *item)
{
    Call *call = g_FSHost.getCallByUUID(item->data(Qt::UserRole).toString());
    QString switch_str = QString("switch %1").arg(call->getCallID());
    QString result;
    if (g_FSHost.sendCmd("pa", switch_str.toAscii(), &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText(QString("Error switching to call %1").arg(call->getCallID()));
        return;
    }
    ui->hangupBtn->setEnabled(true);
}

void MainWindow::makeCall()
{
    bool ok;
    QString dialstring = QInputDialog::getText(this, tr("Make new call"),
                                         tr("Number to dial:"), QLineEdit::Normal, NULL,&ok);

    if (ok && !dialstring.isEmpty())
    {
        paCall(dialstring);
    }
}

void MainWindow::fshostReady()
{
    ui->statusBar->showMessage("Ready", 0);
    ui->newCallBtn->setEnabled(true);
    ui->textEdit->setEnabled(true);
    ui->textEdit->setText("Click on line to dial number...");
}

void MainWindow::paAnswer()
{
    QString result;
    if (g_FSHost.sendCmd("pa", "answer", &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->textEdit->setText("Talking...");
    ui->hangupBtn->setEnabled(true);
    ui->answerBtn->setEnabled(false);
}

void MainWindow::paCall(QString dialstring)
{
    QString result;

    QString callstring = QString("call %1").arg(dialstring);

    if (g_FSHost.sendCmd("pa", callstring.toAscii(), &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->hangupBtn->setEnabled(true);
}

void MainWindow::paHangup()
{
    QString result;
    if (g_FSHost.sendCmd("pa", "hangup", &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->textEdit->setText("Click to dial number...");
    ui->statusBar->showMessage("Call hungup", 10);
    ui->hangupBtn->setEnabled(false);
}

void MainWindow::newOutgoingCall(QString uuid)
{
    Call *call = g_FSHost.getCallByUUID(uuid);
    ui->textEdit->setText(QString("Calling %1 (%2)").arg(call->getCidName(), call->getCidNumber()));
    QListWidgetItem *item = new QListWidgetItem(tr("%1 (%2) - Calling").arg(call->getCidName(), call->getCidNumber()));
    item->setData(Qt::UserRole, uuid);
    ui->listCalls->addItem(item);
    ui->hangupBtn->setEnabled(true);
}

void MainWindow::ringing(QString uuid)
{

    Call *call = g_FSHost.getCallByUUID(uuid);
    ui->textEdit->setText(QString("Call from %1 (%2)").arg(call->getCidName(), call->getCidNumber()));
    QListWidgetItem *item = new QListWidgetItem(tr("%1 (%2) - Ringing").arg(call->getCidName(), call->getCidNumber()));
    item->setData(Qt::UserRole, uuid);
    ui->listCalls->addItem(item);
    ui->answerBtn->setEnabled(true);
}

void MainWindow::answered(QString uuid)
{
    Call *call = g_FSHost.getCallByUUID(uuid);
    for (int i=0; i<ui->listCalls->count(); i++)
    {
        QListWidgetItem *item = ui->listCalls->item(i);
        if (item->data(Qt::UserRole).toString() == uuid)
        {
            item->setText(tr("%1 (%2) - Active").arg(call->getCidName(), call->getCidNumber()));
            break;
        }
    }
}

void MainWindow::hungup(QString uuid)
{
    Call *call = g_FSHost.getCallByUUID(uuid);
    for (int i=0; i<ui->listCalls->count(); i++)
    {
        QListWidgetItem *item = ui->listCalls->item(i);
        if (item->data(Qt::UserRole).toString() == uuid)
        {
            delete ui->listCalls->takeItem(i);
            break;
        }
    }
    ui->textEdit->setText(tr("Call with %1 (%2) hungup.").arg(call->getCidName(), call->getCidNumber()));
    /* TODO: Will cause problems if 2 calls are received at the same time */
    ui->answerBtn->setEnabled(false);
    ui->hangupBtn->setEnabled(false);
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
