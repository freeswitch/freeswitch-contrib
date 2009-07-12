#ifndef SERVER_MANAGER_H
#define SERVER_MANAGER_H

#include <QtGui/QDialog>

namespace Ui {
    class CserverManager;
}

class CserverManager : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(CserverManager)
public:
    explicit CserverManager(QWidget *parent = 0);
    virtual ~CserverManager();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::CserverManager *m_ui;
};

#endif // SERVER_MANAGER_H
