#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&g_FSHost, SIGNAL(ready()),this, SLOT(fshostReady()));
    connect(&g_FSHost, SIGNAL(ringing(QString,QString,QString)), this, SLOT(ringing(QString,QString,QString)));
    connect(ui->answerBtn, SIGNAL(clicked()), this, SLOT(paAnswer()));
    connect(ui->callBtn, SIGNAL(clicked()), this, SLOT(paCall()));
    connect(ui->hangupBtn, SIGNAL(clicked()), this, SLOT(paHangup()));
    connect(ui->paDevlistBtn, SIGNAL(clicked()), this, SLOT(paDevlist()));

    g_FSHost.start();
}

MainWindow::~MainWindow()
{
    delete ui;
    QString res;
    g_FSHost.sendCmd("fsctl", "shutdown", &res);
    g_FSHost.wait();
}

void MainWindow::fshostReady()
{
    ui->statusBar->showMessage("Ready", 0);
    ui->paDevlistBtn->setEnabled(true);
    ui->textEdit->setEnabled(true);
    ui->callBtn->setEnabled(true);
    ui->textEdit->setText("Click to dial number...");
}

void MainWindow::paDevlist()
{
    QString result;
    if (g_FSHost.sendCmd("pa", "devlist as xml", &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->textEdit->setText(result);
}

void MainWindow::paAnswer()
{
    QString result;
    if (g_FSHost.sendCmd("pa", "answer", &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->textEdit->setReadOnly(true);
    ui->textEdit->setText("Talking...");
    ui->hangupBtn->setEnabled(true);
    ui->answerBtn->setEnabled(false);
    ui->callBtn->setEnabled(false);
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
    ui->textEdit->setReadOnly(false);
    ui->callBtn->setEnabled(true);
}

void MainWindow::ringing(QString uuid, QString caller_id_number, QString caller_id_name)
{
    ui->textEdit->setText(QString("Number: %1\nName: %3").arg(caller_id_number, caller_id_name));
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