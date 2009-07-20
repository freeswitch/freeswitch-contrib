#ifndef CONSOLEPAGE_H
#define CONSOLEPAGE_H

#include <QtGui>

namespace Ui {
    class consolePage;
}

/* Forward declarations */
class ESLevent;
class ESLeventLog;
class ESLconnection;
class cmdHistory;
class keyPressEventFilter;

class consolePage : public QWidget {
    Q_OBJECT

public:
    consolePage(QWidget *parent = 0);
    ~consolePage();
    void init(QString host);
    void setConsoleBackground();
    int isConnected();
    void doConnect();
    void doDisconnect();
    QString getConsoleText();

protected:
    virtual void changeEvent(QEvent *e);
    void appendConsoleText(const QString);
signals:
    void gotConnected();
    void gotDisconnected();

private slots:
    void gotConnectedSlot();
    void gotDisconnectedSlot();
    void connectionFailedSlot(QString);
    void gotEventSlot(ESLevent *);
    void gotConsoleEventSlot(ESLeventLog *);
    void getDisconnectedSlot();
    void typedCommand();
    void sendCommand();
    void loglevelChanged(int loglevel);
private:
    void writeSettings();
    void readSettings();
    QAbstractItemModel *modelFromFile(const QString& fileName);
    Ui::consolePage *m_ui;
    ESLconnection *eslConnection;
    cmdHistory *histCompleter;
    keyPressEventFilter *lineCmdEventFilter;
    QString host;
};

class keyPressEventFilter : public QObject {
    Q_OBJECT
public:
    keyPressEventFilter(cmdHistory * histCompleter);
    virtual ~keyPressEventFilter();
protected:
    bool eventFilter(QObject *obj, QEvent *e);
private:
    cmdHistory *histCompleter;
};

#endif // CONSOLEPAGE_H
