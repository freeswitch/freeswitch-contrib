#include <QInputDialog>
#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    dialpadMapper = new QSignalMapper(this);
    connect(ui->dtmf0Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf1Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf2Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf3Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf4Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf5Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf6Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf7Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf8Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmf9Btn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmfAstBtn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    connect(ui->dtmfPoundBtn, SIGNAL(clicked()), dialpadMapper, SLOT(map()));
    dialpadMapper->setMapping(ui->dtmf0Btn, QString("0"));
    dialpadMapper->setMapping(ui->dtmf1Btn, QString("1"));
    dialpadMapper->setMapping(ui->dtmf2Btn, QString("2"));
    dialpadMapper->setMapping(ui->dtmf3Btn, QString("3"));
    dialpadMapper->setMapping(ui->dtmf4Btn, QString("4"));
    dialpadMapper->setMapping(ui->dtmf5Btn, QString("5"));
    dialpadMapper->setMapping(ui->dtmf6Btn, QString("6"));
    dialpadMapper->setMapping(ui->dtmf7Btn, QString("7"));
    dialpadMapper->setMapping(ui->dtmf8Btn, QString("8"));
    dialpadMapper->setMapping(ui->dtmf9Btn, QString("9"));
    dialpadMapper->setMapping(ui->dtmfAstBtn, QString("*"));
    dialpadMapper->setMapping(ui->dtmfPoundBtn, QString("#"));
    connect(dialpadMapper, SIGNAL(mapped(QString)), this, SLOT(dialDTMF(QString)));

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

void MainWindow::dialDTMF(QString dtmf)
{
    QString result;
    QString dtmf_string = QString("dtmf %1").arg(dtmf);
    if (g_FSHost.sendCmd("pa", dtmf_string.toAscii(), &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }
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
    ui->textEdit->setText("Ready to dial and receive calls!");
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
    ui->dtmf0Btn->setEnabled(true);
    ui->dtmf1Btn->setEnabled(true);
    ui->dtmf2Btn->setEnabled(true);
    ui->dtmf3Btn->setEnabled(true);
    ui->dtmf4Btn->setEnabled(true);
    ui->dtmf5Btn->setEnabled(true);
    ui->dtmf6Btn->setEnabled(true);
    ui->dtmf7Btn->setEnabled(true);
    ui->dtmf8Btn->setEnabled(true);
    ui->dtmf9Btn->setEnabled(true);
    ui->dtmfAstBtn->setEnabled(true);
    ui->dtmfPoundBtn->setEnabled(true);
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
    ui->dtmf0Btn->setEnabled(false);
    ui->dtmf1Btn->setEnabled(false);
    ui->dtmf2Btn->setEnabled(false);
    ui->dtmf3Btn->setEnabled(false);
    ui->dtmf4Btn->setEnabled(false);
    ui->dtmf5Btn->setEnabled(false);
    ui->dtmf6Btn->setEnabled(false);
    ui->dtmf7Btn->setEnabled(false);
    ui->dtmf8Btn->setEnabled(false);
    ui->dtmf9Btn->setEnabled(false);
    ui->dtmfAstBtn->setEnabled(false);
    ui->dtmfPoundBtn->setEnabled(false);
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
