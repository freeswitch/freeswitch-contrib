#include "monitorstatemachine.h"
#include "eslconnection.h"
#include "esl.h"

Call::Call(QString orig_uuid, Channel *orig_channel, QString caller_uuid, Channel *caller_channel) :
        _orig_uuid(orig_uuid),
        _orig_channel(orig_channel),
        _caller_uuid(caller_uuid),
        _caller_channel(caller_channel)
{}

MonitorStateMachine::MonitorStateMachine()
{
}

void MonitorStateMachine::addESLconnection(ESLconnection *esl)
{
    _listESL.append(esl);
    connect(esl, SIGNAL(receivedChannelEvent(ESLevent)),
            this, SLOT(processEvent(ESLevent)));
}

void MonitorStateMachine::delESLconnection(ESLconnection *esl)
{
    int i = _listESL.indexOf(esl);
    if (i != -1)
        _listESL.removeAt(i);
}

void MonitorStateMachine::processEvent(ESLevent e)
{
    switch(e.getID())
    {
        case (ESL_EVENT_CHANNEL_CREATE):
        {
            if (_channels.contains(e._headers.value("Unique-ID")))
            {
                qDebug() << "How on earth can we have 2 channels with same UUID? : " << e._headers.value("Unique-ID");
            }
            else
            {
                _channels[e._headers.value("Unique-ID")] = new Channel(e._headers.value("Unique-ID"), e._headers);
                emit channelCreated(_channels.value(e._headers.value("Unique-ID")));
            }
            break;
        }
        case (ESL_EVENT_CHANNEL_BRIDGE) :
        {
            QString orig_uuid = e._headers.value("Other-Leg-Unique-ID");
            QString caller_uuid = e._headers.value("Unique-ID");
            Channel * orig_chan = _channels.value(orig_uuid, NULL);
            Channel * caller_chan = _channels.value(caller_uuid, NULL);
            if (!orig_chan || !caller_chan)
            {
                qDebug() << QString("Channel %1 or %2 do not exist! [ESL_EVENT_CHANNEL_BRIDGE]").arg(orig_uuid, caller_uuid);
            }
            else
            {
                Call * new_call = new Call(orig_uuid, orig_chan, caller_uuid, caller_chan);
                _calls.insert(caller_uuid,new_call);
                caller_chan->stateChanged(e._headers);
                emit callCreated(new_call);
            }
            break;
        }
        case (ESL_EVENT_CHANNEL_UNBRIDGE) :
        {
            QString orig_uuid = e._headers.value("Other-Leg-Unique-ID");
            QString caller_uuid = e._headers.value("Unique-ID");
            Channel * orig_chan = _channels.value(orig_uuid, NULL);
            Channel * caller_chan = _channels.value(caller_uuid, NULL);
            if (!orig_chan || !caller_chan)
            {
                qDebug() << QString("Channel %1 or %2 do not exist! [ESL_EVENT_CHANNEL_UNBRIDGE]").arg(orig_uuid, caller_uuid);
            }
            else
            {
                Call *ecall = _calls.take(caller_uuid);
                _inactive_calls.insert(caller_uuid, ecall);
                caller_chan->stateChanged(e._headers);
                emit callDestroyed(ecall);
            }
            break;
        }
        case (ESL_EVENT_CHANNEL_STATE) :
        {
            if (!_channels.contains(e._headers.value("Unique-ID")))
            {
                qDebug() << "Something went wrong because we do not have this UUID: " << e._headers.value("Unique-ID");
            }
            else
            {
                _channels[e._headers.value("Unique-ID")]->stateChanged(e._headers);
                emit channelStateChanged(_channels.value(e._headers.value("Unique-ID")));
            }
            break;
        }
        case (ESL_EVENT_CHANNEL_DESTROY):
        {
            if (!_channels.contains(e._headers.value("Unique-ID")))
            {
                qDebug() << "Something went wrong because we do not have this UUID: " << e._headers.value("Unique-ID");
            }
            else
            {
                Channel * chan = _channels.take(e._headers.value("Unique-ID"));
                _inactive_channels.insert(e._headers.value("Unique-ID"), chan);
                emit channelDestroyed(chan);
            }
            break;
        }
        default:
        {
            break;
        }
    }

}
