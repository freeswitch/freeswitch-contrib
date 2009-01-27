#include "conn_event_handler.h"


conn_event_handler::conn_event_handler()
{
    memset(&handle, 0, sizeof(esl_handle_t));
}

conn_event_handler::~conn_event_handler(void)
{
    if (isConnected()) {
        Disconnect();
    }
}

void conn_event_handler::Disconnect()
{
    esl_disconnect(&handle);
}

void conn_event_handler::Connect(const QString& hostname, int port, const QString& password) 
{
    
    esl_status_t status = esl_connect(&handle, hostname.toAscii().data(), port, password.toAscii().data());
    
    if ( status != ESL_SUCCESS)
    {
        QString errDesc = handle.err;
        emit connectionError(errDesc);
    }
    else
    {
        emit onConnected();
    }
}

void conn_event_handler::handleError(){
    return;
}

void conn_event_handler::readClient()
{
    return;
}

void conn_event_handler::sendMessage(const QString& message)
{
	if (isConnected())
	{
		esl_status_t status = esl_send_recv(&handle, message.toAscii());
		QString reply;
		if (status != ESL_FAIL)
		{
			if (message.contains("nolog") ||
				message.contains("event") || 
				message.contains("noevent") || 
				message.contains("nixevent") || 
				message.contains("log") || 
				message.contains("nolog") || 
				message.contains("filter")
				)
			{
				reply = handle.last_sr_reply;
				reply+="\n";
			}
			else if (message.contains("exit"))
			{
				emit connectionError("Disconnected!");
				handle.connected = ESL_FALSE;
				reply = "See you!\n";
			}
			else
			{
				reply = handle.last_sr_event->body;
			}
		}
		emit messageSignal(reply);
	}
    return;
}

void conn_event_handler::handleRecvMessage(const QString &msg)
{
}

bool conn_event_handler::isConnected() const
{
    return handle.connected;
}
