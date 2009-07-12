#include "fs_gui.h"
#include "ui_fs_gui.h"

Cfsgui::Cfsgui(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::Cfsgui),
    serverDialog(new CserverManager)
{
    m_ui->setupUi(this);
}

Cfsgui::~Cfsgui()
{
    delete m_ui;
    delete serverDialog;
}

void Cfsgui::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Cfsgui::closeEvent(QCloseEvent *e)
{
    /* TODO: We have to stop threads and do cleanup */
    e->accept();
}
