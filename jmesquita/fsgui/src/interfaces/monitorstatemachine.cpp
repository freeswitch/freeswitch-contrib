#include "monitorstatemachine.h"
#include "eslconnection.h"
#include "esl.h"

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
                _channels[e._headers.value("Unique-ID")]->stateChanged(e._headers);
                emit channelDestroyed(_channels.value(e._headers.value("Unique-ID")));
            }
            break;
        }
        default:
        {
            if (e.getType() != "COMMAND")
                qDebug() << "Event not treated: " << e.getType();
        }
    }

}
