#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->fshost = new FSHost();

    connect(this->fshost, SIGNAL(ready()),this, SLOT(fshostReady()));
    connect(ui->paDevlistBtn, SIGNAL(clicked()), this, SLOT(paDevlist()));
    connect(ui->showCodecsBtn, SIGNAL(clicked()), this, SLOT(showCodecs()));

    this->fshost->start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::fshostReady()
{
    ui->statusBar->showMessage("FreeSWITCH is ready!", 0);
}

void MainWindow::paDevlist()
{
    QString result;
    if (this->fshost->sendCmd("pa", "devlist as xml", &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->textEdit->setText(result);
}

void MainWindow::showCodecs()
{
    QString result;
    if (this->fshost->sendCmd("show", "codecs as xml", &result) == SWITCH_STATUS_FALSE) {
        ui->textEdit->setText("Error sending that command");
    }

    ui->textEdit->setText(result);
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
