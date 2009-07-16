#ifndef CONSOLEPAGE_H
#define CONSOLEPAGE_H

#include <QtGui/QWidget>

namespace Ui {
    class consolePage;
}

/* Forward declarations */
class ESLevent;
class ESLeventLog;
class ESLconnection;

class consolePage : public QWidget {
    Q_OBJECT

public:
    consolePage(QWidget *parent = 0);
    ~consolePage();
    void init(QString host);

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
    Ui::consolePage *m_ui;
    ESLconnection *eslConnection;
    QString host;
};

#endif // CONSOLEPAGE_H
