#ifndef ESL_CONNECTION_H
#define ESL_CONNECTION_H

#include <QThread>

/* Forward declaration of ESLConnection */
class ESLconnection;
class ESLevent;

class eslConnectionManager : public QThread    
{
    Q_OBJECT
    Q_DISABLE_COPY(eslConnectionManager)
public:
    eslConnectionManager();
    eslConnectionManager(QString,QString,QString);
    void doConnect(QString, QString, QString);
    void doDisconnect();
signals:
    void gotDisconnected();
    void gotConnected();
    void gotEvent(ESLevent *);
private:
    ESLconnection* connection;
    QString _host;
    QString _pass;
    QString _port;
    bool isConnected;

protected:
    void run();
};

#endif // ESL_CONNECTION_H
