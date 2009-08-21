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

    /* Connect event stuff */
    connect(sm, SIGNAL(newEvent(Channel*,Event*)),
            this, SLOT(newEvent(Channel*,Event*)));
    connect(m_ui->listActiveEvents, SIGNAL(clicked(QModelIndex)),
            this, SLOT(activeEventSelected(QModelIndex)));
    connect(m_ui->listInactiveEvents, SIGNAL(clicked(QModelIndex)),
            this, SLOT(inactiveEventSelected(QModelIndex)));

    /* Connect channel stuff */
    connect(sm, SIGNAL(channelCreated(Channel*)),
            this, SLOT(channelCreate(Channel*)));
    connect(sm, SIGNAL(channelDestroyed(Channel*)),
            this, SLOT(channelDestroy(Channel*)));
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

    /* Channel models */
    _channel_model = new QStandardItemModel(this);
    _channel_sort_model = new ChannelSortModel(this);
    _channel_sort_model->setSourceModel(_channel_model);
    m_ui->listActiveChannels->setModel(_channel_sort_model);

    _inactive_channel_model = new QStandardItemModel(this);
    _inactive_channel_sort_model = new ChannelSortModel(this);
    _inactive_channel_sort_model->setSourceModel(_inactive_channel_model);
    m_ui->listInactiveChannels->setModel(_inactive_channel_sort_model);

    /* Event models */
    _event_model = new QStandardItemModel(this);
    _event_sort_model = new EventSortModel(this);
    _event_sort_model->setSourceModel(_event_model);
    m_ui->listActiveEvents->setModel(_event_sort_model);
    m_ui->listInactiveEvents->setModel(_event_sort_model);

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

    QStandardItem *item = new QStandardItem(QString("%1").arg(ch->getUUID()));
    item->setData(ch->getUUID(), Qt::UserRole);
    _channel_model->appendRow(item);
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
            {
                m_ui->listActiveHeaders->clear();
            }

            _channel_model->takeRow(item->index().row());
            _inactive_channel_model->appendRow(item);
            _event_sort_model->setUUIDFilter("");
        }
    }
}

void RealtimeStatisticsDialog::activeChannelSelected(QModelIndex index)
{

    m_ui->listActiveHeaders->clear();

    Channel * ch = _sm->getChannel(index.data(Qt::UserRole).toString());
    if (!ch)
        return;

    _event_sort_model->setUUIDFilter(ch->getUUID());
}

void RealtimeStatisticsDialog::inactiveChannelSelected(QModelIndex index)
{
    m_ui->listActiveHeaders->clear();

    Channel * ch = _sm->getInactiveChannel(index.data(Qt::UserRole).toString());
    if (!ch)
        return;

    _event_sort_model->setUUIDFilter(ch->getUUID());
}

void RealtimeStatisticsDialog::activeEventSelected(QModelIndex index)
{
    Channel *ch = _sm->getChannel(index.data(Qt::UserRole).toString());
    if (!ch)
        return;

    m_ui->listActiveHeaders->clear();

    Event * e = ch->getEvent(index.row());
    foreach (QString key, e->getHeaders().keys())
    {
        m_ui->listActiveHeaders->addItem(QString("%1 = %2").arg(key, e->getHeaders().value(key)));
    }
}

void RealtimeStatisticsDialog::inactiveEventSelected(QModelIndex index)
{
    Channel *ch = _sm->getInactiveChannel(index.data(Qt::UserRole).toString());
    if (!ch)
        return;

    m_ui->listInactiveHeaders->clear();

    Event * e = ch->getEvent(index.row());
    foreach (QString key, e->getHeaders().keys())
    {
        m_ui->listInactiveHeaders->addItem(QString("%1 = %2").arg(key, e->getHeaders().value(key)));
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
    QString caller_cidnum = c->getCallerChannel()->getCurrentStateHeader("Caller-Caller-ID-Number");
    QString caller_cidname = c->getCallerChannel()->getCurrentStateHeader("variable_effective_caller_id_name");
    QString orig_cidnum = c->getCallerChannel()->getCurrentStateHeader("Caller-Destination-Number");
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

void RealtimeStatisticsDialog::newEvent(Channel *ch, Event *e)
{
    if (!ch && !e)
        return;

    QStandardItem *item = new QStandardItem(e->getEventName());
    item->setData(ch->getUUID(), Qt::UserRole);
    _event_model->appendRow(item);
}
