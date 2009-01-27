#include "fsgui.h"
#include "ui_fsgui.h"
#include "serverlogin.h"

FSGui::FSGui(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::FSGuiClass)
{
    ui->setupUi(this);
    serverLogin = new ServerLogin(this);
    serverLogin->show();

    connect( serverLogin , SIGNAL(acceptedSignal(QString, QString, QString)),
             this, SLOT(connectSlot(QString, QString, QString)) );

    connect(ui->btnSend, SIGNAL(clicked()), this, SLOT(btnSendClickedSlot()));
    connect(ui->lineCmd, SIGNAL(textChanged(QString)), this, SLOT(lineCmdEditedSlot(QString)));
    connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionConnect, SIGNAL(triggered()), serverLogin, SLOT(show()));

}

FSGui::~FSGui()
{
    delete socket;
    delete ui;
}

void FSGui::error(QString msg){
    ui->statusBar->showMessage(msg);
}

void FSGui::connectSlot(QString hostname, QString port, QString password){
    socket = new conn_event_handler();
    connect( socket  , SIGNAL( connectionError(QString) ),
             this, SLOT( error(QString) ) );
    connect(socket , SIGNAL( gotConnected() ),
            this, SLOT(connectionSuccessful()));
    connect(socket , SIGNAL( messageSignal(QString) ),
            this, SLOT( messageSlot(QString) ));
    connect(this, SIGNAL(btnSendClickedSignal(QString)),
            socket, SLOT(sendMessage(QString)));
    socket->getConnected(hostname, port.toInt(), password);
    ui->btnSend->setEnabled(true);
}

void FSGui::connectionSuccessful(void){
    ui->statusBar->showMessage(tr("Connected!"));
}

void FSGui::btnSendClickedSlot()
{
    emit btnSendClickedSignal(ui->lineCmd->text());
    ui->lineCmd->clear();
}

void FSGui::lineCmdEditedSlot(QString text)
{
    if (socket->isConnected() && !text.isEmpty())
    {
        ui->btnSend->setEnabled(true);
    }
    else
    {
        ui->btnSend->setEnabled(false);
    }
}

void FSGui::messageSlot(QString msg)
{
    if (!msg.isEmpty())
    {
        QTime timestamp = QTime::currentTime();
        QStringList msgList = msg.split("\n", QString::SkipEmptyParts);
        for (int i = 0 ; i < msgList.size() ; ++i)
        {
            QString message = timestamp.toString("hh:mm:ss.zzz") + " : " + msgList[i] + "\n";
            ui->textConsole->insertPlainText(message);
        }
    }
}
