#include <QtGui>
#include "realtimestatisticsdialog.h"
#include "ui_realtimestatisticsdialog.h"
#include "monitorstatemachine.h"

RealtimeStatisticsDialog::RealtimeStatisticsDialog(QWidget *parent, MonitorStateMachine *sm) :
    QDialog(parent),
    m_ui(new Ui::RealtimeStatisticsDialog),
    _sm(sm)
{
    m_ui->setupUi(this);
    connect(sm, SIGNAL(channelCreated(Channel*)),
            this, SLOT(channelCreate(Channel*)));
    connect(sm, SIGNAL(channelDestroyed(Channel*)),
            this, SLOT(channelDestroy(Channel*)));
    connect(m_ui->listActiveChannels, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(channelSelected(QListWidgetItem*)));
    delete m_ui->tab_2;
}

RealtimeStatisticsDialog::~RealtimeStatisticsDialog()
{
    delete m_ui;
}

void RealtimeStatisticsDialog::changeEvent(QEvent *e)
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

void RealtimeStatisticsDialog::channelCreate(Channel *ch)
{
    QListWidgetItem *item = new QListWidgetItem(QString("%1 - %2").arg(ch->getUUID(), ch->getHeader("Channel-State")),
                                                m_ui->listActiveChannels);
    item->setData(Qt::UserRole, ch->getUUID());
}

void RealtimeStatisticsDialog::channelDestroy(Channel *ch)
{
    for (int i = 0; i < m_ui->listActiveChannels->count(); i++)
    {
        if (m_ui->listActiveChannels->item(i)->data(Qt::UserRole) == ch->getUUID())
        {
            if (m_ui->listActiveChannels->currentItem() == m_ui->listActiveChannels->item(i))
                m_ui->listActiveVariables->clear();

            delete m_ui->listActiveChannels->item(i);
        }
    }
}

void RealtimeStatisticsDialog::channelSelected(QListWidgetItem *item)
{
    m_ui->listActiveVariables->clear();

    Channel * ch = _sm->getChannel(item->data(Qt::UserRole).toString());
    if (!ch)
        return;

    foreach (QString key, ch->getVariables().keys())
    {
        m_ui->listActiveVariables->addItem(QString("%1 = %2").arg(key, ch->getVariables().value(key)));
    }
}
