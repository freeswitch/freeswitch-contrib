#include "monitorstatemachine.h"
#include "eslconnection.h"
#include "esl.h"

State::State(int state_id, QString state_name, QHash<QString, QString> variables)
        : _state_id(state_id),
        _state_name(state_name),
        _variables(variables)
{}

Channel::Channel(QString uuid, State *state)
        : _uuid(new QString(uuid))
{
    _states.append(state);
    _currentConfig = state;
}

Call::Call(Channel *orig_channel, Channel *caller_channel) :
        _orig_channel(orig_channel),
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
                State *state = new State(e._headers.value("Channel-State-Number").toInt(), e._headers.value("Channel-State"), e._headers);
                _channels[e._headers.value("Unique-ID")] = new Channel(e._headers.value("Unique-ID"), state);
                emit channelCreated(_channels.value(e._headers.value("Unique-ID")));
            }
            break;
        }
        case (ESL_EVENT_CHANNEL_UUID):
        {
            QString uuid = e._headers.value("Old-Unique-ID");
            QString new_uuid = e._headers.value("Unique-ID");
            Channel * chan = _channels.value(uuid, NULL);
            if (chan)
            {
                chan->setUUID(new_uuid);
                if(_calls.contains(uuid))
                {
                    Call *call = _calls.take(uuid);
                    _calls.insert(new_uuid, call);
                }
                else if (_inactive_calls.contains(uuid))
                {
                    /* This is quite impossible anyway ... */
                    Call *call = _inactive_calls.take(uuid);
                    _inactive_calls.insert(new_uuid, call);
                }
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
                State *state = new State(e._headers.value("Channel-State-Number").toInt(), e._headers.value("Channel-State"), e._headers);
                Call * new_call = new Call(orig_chan, caller_chan);
                _calls.insert(caller_uuid,new_call);
                caller_chan->stateChanged(state);
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
                State *state = new State(e._headers.value("Channel-State-Number").toInt(), e._headers.value("Channel-State"), e._headers);
                Call *ecall = _calls.take(caller_uuid);
                _inactive_calls.insert(caller_uuid, ecall);
                caller_chan->stateChanged(state);
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
                State *state = new State(e._headers.value("Channel-State-Number").toInt(), e._headers.value("Channel-State"), e._headers);
                _channels[e._headers.value("Unique-ID")]->stateChanged(state);
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
                State *state = new State(e._headers.value("Channel-State-Number").toInt(), e._headers.value("Channel-State"), e._headers);
                Channel * chan = _channels.take(e._headers.value("Unique-ID"));
                _inactive_channels.insert(e._headers.value("Unique-ID"), chan);
                chan->stateChanged(state);
                qDebug() << chan->getStates();
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
