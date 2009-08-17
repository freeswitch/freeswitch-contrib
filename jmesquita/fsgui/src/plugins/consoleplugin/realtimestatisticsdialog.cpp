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

    /* Connect channel stuff */
    connect(sm, SIGNAL(channelCreated(Channel*)),
            this, SLOT(channelCreate(Channel*)));
    connect(sm, SIGNAL(channelDestroyed(Channel*)),
            this, SLOT(channelDestroy(Channel*)));
    connect(sm, SIGNAL(channelStateChanged(Channel*)),
            this, SLOT(channelStateChanged(Channel*)));
    connect(m_ui->listActiveChannels, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(channelSelected(QListWidgetItem*)));

    /* Connect call stuff */
    connect(sm, SIGNAL(callCreated(Call*)),
            this, SLOT(callCreate(Call*)));
    connect(sm, SIGNAL(callDestroyed(Call*)),
            this, SLOT(callDestroy(Call*)));

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
    /* Sanity checking */
    if (!ch)
        return;

    QListWidgetItem *item = new QListWidgetItem(QString("%1 - %2").arg(ch->getUUID(), ch->getHeader("Channel-State")),
                                                m_ui->listActiveChannels);
    item->setData(Qt::UserRole, ch->getUUID());
}

void RealtimeStatisticsDialog::channelDestroy(Channel *ch)
{
    /* Sanity checking */
    if (!ch)
        return;

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

void RealtimeStatisticsDialog::channelStateChanged(Channel * ch)
{
    /* Sanity checking */
    if (!ch)
        return;

    for (int i = 0; i < m_ui->listActiveChannels->count(); i++)
    {
        if (m_ui->listActiveChannels->item(i)->data(Qt::UserRole).toString() == ch->getUUID())
            if (m_ui->listActiveChannels->currentItem() == m_ui->listActiveChannels->item(i))
            {
                channelSelected(m_ui->listActiveChannels->item(i));
            }
        m_ui->listActiveChannels->item(i)->setText(QString("%1 - %2").arg(ch->getUUID(), ch->getHeader("Channel-State")));
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

void RealtimeStatisticsDialog::callCreate(Call *c)
{
    /* Sanity checking */
    if (!c)
        return;
    QString caller_cidnum = c->getCallerChannel()->getHeader("Caller-Caller-ID-Number");
    QString caller_cidname = c->getCallerChannel()->getHeader("variable_effective_caller_id_name");
    QString orig_cidnum = c->getCallerChannel()->getHeader("Caller-Destination-Number");
    QListWidgetItem *item = new QListWidgetItem(QString("%1(%2) <-> %3").arg(caller_cidnum,
                                                                             caller_cidname,
                                                                             orig_cidnum), m_ui->listActiveCalls);
    item->setData(Qt::UserRole, c->getCallerUUID());
}

void RealtimeStatisticsDialog::callDestroy(Call *c)
{
    /* Sanity checking */
    if (!c)
        return;

    for (int i = 0; i < m_ui->listActiveCalls->count(); i++)
    {
        if (m_ui->listActiveCalls->item(i)->data(Qt::UserRole) == c->getCallerUUID())
            delete m_ui->listActiveCalls->item(i);
    }
}
