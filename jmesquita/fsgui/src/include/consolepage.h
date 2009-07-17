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

class consolePage : public QWidget {
    Q_OBJECT

public:
    consolePage(QWidget *parent = 0);
    ~consolePage();
    void init(QString host);
    void setConsoleBackground();

protected:
    virtual void changeEvent(QEvent *e);
    void appendConsoleText(const QString);

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
    QString host;
    cmdHistory *histCompleter;

};

#endif // CONSOLEPAGE_H
