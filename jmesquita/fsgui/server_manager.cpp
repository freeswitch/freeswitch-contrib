#include "server_manager.h"
#include "ui_server_manager.h"

CserverManager::CserverManager(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::CserverManager)
{
    m_ui->setupUi(this);
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
