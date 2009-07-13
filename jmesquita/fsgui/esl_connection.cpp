#include "esl_oop.h"
#include "esl_connection.h"
#include <iostream>

eslConnectionManager::eslConnectionManager() {}
eslConnectionManager::eslConnectionManager(QString host, QString pass, QString port)
        : _host(host), _pass(pass), _port(port)
{
    doConnect(host, pass, port);
    start();
}

eslConnectionManager::~eslConnectionManager()
{
    doDisconnect();
}

void eslConnectionManager::doConnect(QString host, QString pass, QString port)
{
    connection = new ESLconnection(host.toAscii(), port.toAscii(), pass.toAscii());
    eslSetLogLevel(0);
    isConnected = false;
}

void eslConnectionManager::doDisconnect()
{
    delete connection;
}

void eslConnectionManager::run()
{
    forever
    {
        if (!connection->connected() && isConnected)
        {
            isConnected = false;
            emit gotDisconnected();
        }
        else if (connection->connected() && !isConnected)
        {
            isConnected = true;
            connection->sendRecv("log 7");
            emit gotConnected();
        }
        else if (connection->connected() && isConnected)
        {
            /* We should process our event queue here */
            ESLevent * event = connection->recvEventTimed(10);
            if (event)
            {
                ESLevent * e = new ESLevent(event);
                emit gotEvent(e);
            }
         }
        else
        {
            doDisconnect();
            break;
        }
    }
}
