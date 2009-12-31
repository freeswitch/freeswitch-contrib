#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(&g_FSHost, SIGNAL(ready()),this, SLOT(fshostReady()));
    connect(&g_FSHost, SIGNAL(ringing(QString,QString,QString)), this, SLOT(ringing(QString,QString,QString)));
    connect(ui->paDevlistBtn, SIGNAL(clicked()), this, SLOT(paDevlist()));
    connect(ui->showCodecsBtn, SIGNAL(clicked()), this, SLOT(showCodecs()));

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
    ui->statusBar->showMessage("FreeSWITCH is ready!", 0);
}

void MainWindow::paDevlist()
{
    QString result;
    if (g_FSHost.sendCmd("pa", "devlist as xml", &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->textEdit->setText(result);
}

void MainWindow::showCodecs()
{
    QString result;
    if (g_FSHost.sendCmd("show", "codecs as xml", &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->textEdit->setText(result);
}

void MainWindow::ringing(QString uuid, QString caller_id_number, QString caller_id_name)
{
    ui->textEdit->setText(QString("call_id: %1, Number: %2, Name: %3").arg(uuid, caller_id_number, caller_id_name));
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
