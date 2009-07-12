#ifndef FS_GUI_H
#define FS_GUI_H

#include <QtGui>
#include "server_manager.h"

namespace Ui {
    class Cfsgui;
}

class Cfsgui : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY(Cfsgui)
public:
    explicit Cfsgui (QWidget *parent = 0);
    virtual ~Cfsgui();

protected:
    virtual void changeEvent(QEvent *e);
    void closeEvent(QCloseEvent *e);

private:
    Ui::Cfsgui *m_ui;
    CserverManager *serverDialog;
};

#endif // FS_GUI_H
