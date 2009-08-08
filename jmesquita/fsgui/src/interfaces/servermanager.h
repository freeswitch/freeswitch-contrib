#ifndef SERVERMANAGER_H
#define SERVERMANAGER_H

#include <QtGui/QDialog>

namespace Ui {
    class ServerManager;
}

class ESLconnection;

class QTreeWidgetItem;

class ServerManager : public QDialog {
    Q_OBJECT
public:
    ServerManager(QWidget *parent = 0);
    ~ServerManager();
    ESLconnection * getESLconnection();

protected:
    void changeEvent(QEvent *e);

private slots:
    void currentItemChanged ( QTreeWidgetItem * current, QTreeWidgetItem * previous );
    void addServer();
    void editServer();
    void saveServer();
    void deleteServer();

private:
    Ui::ServerManager *m_ui;

    enum {
        Host = Qt::UserRole,
        Port,
        Password,
        Name
    };

    void readSettings();
};

#endif // SERVERMANAGER_H
