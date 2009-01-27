#include "conn_event_handler.h"


conn_event_handler::conn_event_handler()
{
    memset(&handle, 0, sizeof(esl_handle_t));
}

conn_event_handler::~conn_event_handler(void){
    getDisconnected();
	delete &handle;
}

void conn_event_handler::getDisconnected(){
    esl_disconnect(&handle);
}

void conn_event_handler::getConnected(QString hostname, int port, QString password){
    QByteArray h = hostname.toAscii();
    QByteArray p = password.toAscii();
    const char * Hostname = h.data();
    const char * Password = p.data();
	esl_status_t status = esl_connect(&handle, Hostname, port, Password);
	if ( status != ESL_SUCCESS)
	{
            QString errDesc = handle.err;
            emit connectionError(errDesc);
	}
	else
	{
            emit gotConnected();
	}
}

void conn_event_handler::handleError(){
    return;
}

void conn_event_handler::readClient()
{
    return;
}

void conn_event_handler::sendMessage(QString message)
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

void conn_event_handler::handleRecvMessage(QString msg)
{
    return;
}

bool conn_event_handler::isConnected()
{
    return handle.connected;
}
