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
        if (connection->connected() && isConnected)
        {
            /* We should process our event queue here */
            ESLevent * event = connection->recvEventTimed(10);
            if (event)
            {
                std::cout << "We received and event!" << std::endl;
                emit gotEvent(event);
            }
         }
        else if (!connection->connected() && isConnected)
        {
            std::cout << "We just got disconnected." << std::endl;
            isConnected = false;
            emit gotDisconnected();
        }
        else if (connection->connected() && !isConnected)
        {
            std::cout << "We just got connected." << std::endl;
            isConnected = true;
            connection->sendRecv("log 7");
            emit gotConnected();
        }
        else
        {
            std::cout << "We are disconnected and should stop this." << std::endl;
            doDisconnect();
            break;
        }
    }
}
