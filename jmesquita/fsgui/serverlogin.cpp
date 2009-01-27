#include "serverlogin.h"
#include "ui_serverlogin.h"

ServerLogin::ServerLogin(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::ServerLogin)
{
    m_ui->setupUi(this);
    this->setModal(true);
    
    connect (this, SIGNAL(accepted()), this, SLOT(acceptedSlot()));
}

ServerLogin::~ServerLogin()
{
    delete m_ui;
}

void ServerLogin::changeEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ServerLogin::acceptedSlot()
{
    emit acceptedSignal(m_ui->editHostname->text(), m_ui->editPort->text(), m_ui->editPassword->text());
}
