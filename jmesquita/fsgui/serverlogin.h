#ifndef SERVERLOGIN_H
#define SERVERLOGIN_H

#include <QtGui/QDialog>

namespace Ui {
    class ServerLogin;
}

class ServerLogin : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(ServerLogin)
public:
    explicit ServerLogin(QWidget *parent = 0);
    virtual ~ServerLogin();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::ServerLogin *m_ui;

private slots:
    void acceptedSlot();
    
signals:
    void acceptedSignal(QString, QString, QString);
};

#endif // SERVERLOGIN_H
