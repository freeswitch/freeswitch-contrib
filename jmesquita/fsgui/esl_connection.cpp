#include <QtGui>
#include "esl_oop.h"
#include "esl_connection.h"

eslConnectionManager::eslConnectionManager() {}
eslConnectionManager::eslConnectionManager(QString host, QString pass, QString port)
        : _host(host), _pass(pass), _port(port)
{
    doConnect(host, pass, port);
    start();
}

void eslConnectionManager::doConnect(QString host, QString pass, QString port)
{
    connection = new ESLconnection(host.toAscii(), port.toAscii(), pass.toAscii());
    eslSetLogLevel(7);
    isConnected = false;
}

void eslConnectionManager::doDisconnect()
{
        connection->disconnect();
}

void eslConnectionManager::sendCmd(QString string)
{
    if (connection->connected())
    {
        QStringList list = string.split(" ", QString::SkipEmptyParts, Qt::CaseSensitive);
        QString cmd = list.takeFirst();
        QString arg("");
        for (int i=0; i < list.size(); i++)
        {
            arg.append(list[i]);
            if (i != list.size()-1)
                arg.append(" ");
        }
        connection->api(cmd.toAscii(), arg.toAscii());
    }
}

void eslConnectionManager::run()
{
    forever
    {
        if (!connection->connected() && isConnected)
        {
            isConnected = false;
            emit gotDisconnected();
            break;
        }
        else if (connection->connected() && !isConnected)
        {
            isConnected = true;
            connection->sendRecv("log 7");
            emit gotConnected();
        }
        else if (connection->connected() && isConnected)
        {
            ESLevent * event = connection->recvEventTimed(10);
            if (event)
            {
                ESLevent * e = new ESLevent(event);
                emit gotEvent(e);
            }
         }
    }
}
