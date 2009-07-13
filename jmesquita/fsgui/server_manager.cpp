#include "server_manager.h"
#include "ui_server_manager.h"

CserverManager::CserverManager(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CserverManager)
{
    m_ui->setupUi(this);
    connect(this, SIGNAL(accepted()),
            this, SLOT(newConnection()));
}

CserverManager::~CserverManager()
{
    delete m_ui;
}

void CserverManager::changeEvent(QEvent *e)
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

void CserverManager::newConnection()
{
    //TODO: Sanity checking needed.
    emit doConnect(m_ui->host->text(),
                   m_ui->pass->text(),
                   m_ui->port->text());
}
