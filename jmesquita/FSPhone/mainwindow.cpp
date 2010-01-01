#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&g_FSHost, SIGNAL(ready()),this, SLOT(fshostReady()));
    connect(&g_FSHost, SIGNAL(ringing(Call *)), this, SLOT(ringing(Call *)));
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
    Call *call = _lines.value(item->data(Qt::UserRole).toString());
    QString switch_str = QString("switch %1").arg(call->getCallID());
    QString result;
    if (g_FSHost.sendCmd("pa", switch_str.toAscii(), &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText(QString("Error switching to call %1").arg(call->getCallID()));
        return;
    }
    ui->hangupBtn->setEnabled(true);
}

void MainWindow::fshostReady()
{
    ui->statusBar->showMessage("Ready", 0);
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

void MainWindow::paCall()
{
    QString result;

    QString callstring = QString("call %1").arg(ui->textEdit->toPlainText());

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

void MainWindow::ringing(Call *call)
{
    if (_lines.contains(call->getUUID()))
    {
        delete call;
    }
    else
    {
        _lines.insert(call->getUUID(), call);
        ui->textEdit->setText(QString("Number: %1\nName: %3").arg(call->getCidNumber(), call->getCidName()));
        QListWidgetItem *item = new QListWidgetItem(tr("%1 - %2 (Ringing)").arg(call->getCidName(), call->getCidNumber()), ui->listCalls);
        item->setData(Qt::UserRole, call->getUUID());
    }
    ui->answerBtn->setEnabled(true);
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
