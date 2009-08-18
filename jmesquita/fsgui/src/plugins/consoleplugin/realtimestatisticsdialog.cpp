#include <QtGui>
#include "realtimestatisticsdialog.h"
#include "ui_realtimestatisticsdialog.h"
#include "monitorstatemachine.h"
#include "realtimemodels.h"

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
    connect(m_ui->listActiveChannels, SIGNAL(clicked(QModelIndex)),
            this, SLOT(activeChannelSelected(QModelIndex)));
    connect(m_ui->listInactiveChannels, SIGNAL(clicked(QModelIndex)),
            this, SLOT(inactiveChannelSelected(QModelIndex)));

    /* Connect call stuff */
    connect(sm, SIGNAL(callCreated(Call*)),
            this, SLOT(callCreate(Call*)));
    connect(sm, SIGNAL(callDestroyed(Call*)),
            this, SLOT(callDestroy(Call*)));
    connect(m_ui->listActiveCalls, SIGNAL(itemSelectionChanged()),
            this, SLOT(activeCallSelectionChanged()));
    connect(m_ui->listInactiveCalls, SIGNAL(itemSelectionChanged()),
            this, SLOT(inactiveCallSelectionChanged()));

    _channel_model = new QStandardItemModel(this);
    _channel_sort_model = new ChannelSortModel(this);
    _channel_sort_model->setSourceModel(_channel_model);
    m_ui->listActiveChannels->setModel(_channel_sort_model);

    _inactive_channel_model = new QStandardItemModel(this);
    _inactive_channel_sort_model = new ChannelSortModel(this);
    _inactive_channel_sort_model->setSourceModel(_inactive_channel_model);
    m_ui->listInactiveChannels->setModel(_inactive_channel_sort_model);

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

    QStandardItem *item = new QStandardItem(QString("%1 - %2").arg(ch->getUUID(), ch->getHeader("Channel-State")));
    item->setData(ch->getUUID(), Qt::UserRole);
    _channel_model->insertRow(0, item);
}

void RealtimeStatisticsDialog::channelDestroy(Channel *ch)
{
    /* Sanity checking */
    if (!ch)
        return;

    QModelIndex selectedIndex = m_ui->listActiveChannels->currentIndex();

    foreach (QStandardItem *item, _channel_model->findItems("*", Qt::MatchWildcard | Qt::MatchRecursive))
    {
        if (item->data(Qt::UserRole) == ch->getUUID())
        {
            if (selectedIndex.isValid() && selectedIndex.data(Qt::UserRole) == ch->getUUID())
                m_ui->listActiveVariables->clear();

            _channel_model->takeRow(item->index().row());
            _inactive_channel_model->insertRow(0, item);
        }
    }
}

void RealtimeStatisticsDialog::channelStateChanged(Channel * ch)
{
    /* Sanity checking */
    if (!ch)
        return;

    foreach (QStandardItem *item, _channel_model->findItems("*", Qt::MatchWildcard | Qt::MatchRecursive))
    {
        if (item->data(Qt::UserRole) == ch->getUUID())
            if (_channel_model->itemFromIndex(m_ui->listActiveChannels->currentIndex()) == item)
            {
                activeChannelSelected(_channel_model->indexFromItem(item));
            }
        item->setText(QString("%1 - %2").arg(ch->getUUID(), ch->getHeader("Channel-State")));
    }
}

void RealtimeStatisticsDialog::activeChannelSelected(QModelIndex index)
{
    m_ui->listActiveVariables->clear();

    Channel * ch = _sm->getChannel(index.data(Qt::UserRole).toString());
    if (!ch)
        return;

    foreach (QString key, ch->getVariables().keys())
    {
        m_ui->listActiveVariables->addItem(QString("%1 = %2").arg(key, ch->getVariables().value(key)));
    }
}

void RealtimeStatisticsDialog::inactiveChannelSelected(QModelIndex index)
{
    m_ui->listInactiveVariables->clear();

    Channel * ch = _sm->getInactiveChannel(index.data(Qt::UserRole).toString());
    if (!ch)
        return;

    foreach (QString key, ch->getVariables().keys())
    {
        m_ui->listInactiveVariables->addItem(QString("%1 = %2").arg(key, ch->getVariables().value(key)));
    }
}

void RealtimeStatisticsDialog::activeCallSelectionChanged()
{
    QList<QListWidgetItem *> selection = m_ui->listActiveCalls->selectedItems();
    QList<QString> uuids;

    foreach(QListWidgetItem *item, selection)
    {
        Call * call = _sm->getCall(item->data(Qt::UserRole).toString());
        if (call)
        {
            uuids.append(call->getCallerUUID());
            uuids.append(call->getOriginateeUUID());
        }
    }
    _channel_sort_model->setUUIDFilter(uuids);
}

void RealtimeStatisticsDialog::inactiveCallSelectionChanged()
{
    QList<QListWidgetItem *> selection = m_ui->listInactiveCalls->selectedItems();
    QList<QString> uuids;

    foreach(QListWidgetItem *item, selection)
    {
        Call * call = _sm->getInactiveCall(item->data(Qt::UserRole).toString());
        if (call)
        {
            uuids.append(call->getCallerUUID());
            uuids.append(call->getOriginateeUUID());
        }
    }
    _inactive_channel_sort_model->setUUIDFilter(uuids);
}

void RealtimeStatisticsDialog::callCreate(Call *c)
{
    /* Sanity checking */
    if (!c)
        return;
    QString caller_cidnum = c->getCallerChannel()->getHeader("Caller-Caller-ID-Number");
    QString caller_cidname = c->getCallerChannel()->getHeader("variable_effective_caller_id_name");
    QString orig_cidnum = c->getCallerChannel()->getHeader("Caller-Destination-Number");
    QString caller_uuid = c->getCallerUUID();
    QString callTitle = QString("%1(%2) <-> %3 (Caller UUID: %4)").arg(caller_cidnum, caller_cidname, orig_cidnum, caller_uuid);
    QListWidgetItem *item = new QListWidgetItem(callTitle, m_ui->listActiveCalls);
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
        {
            QListWidgetItem *item = m_ui->listActiveCalls->takeItem(i);
            m_ui->listInactiveCalls->addItem(item);
        }
    }
}
