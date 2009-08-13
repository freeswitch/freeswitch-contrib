#include <QtGui>
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::SettingsDialog)
{
    m_ui->setupUi(this);
    settings = new QSettings;
    QObject::connect(m_ui->listConfig, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
                     this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}

void SettingsDialog::changeEvent(QEvent *e)
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

void SettingsDialog::addConfigItem(QListWidgetItem *item, QWidget *page)
{
    m_ui->listConfig->addItem(item);
    m_ui->configPages->insertWidget(m_ui->listConfig->row(item),page);
}

void SettingsDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    m_ui->configPages->setCurrentIndex(m_ui->listConfig->row(current));
}

void SettingsDialog::removeConfigItem(QListWidgetItem *item, QWidget *page)
{
    m_ui->listConfig->removeItemWidget(item);
    m_ui->configPages->removeWidget(page);
    delete item;
    delete page;
}
