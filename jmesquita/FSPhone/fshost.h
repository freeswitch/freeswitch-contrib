#ifndef FSHOST_H
#define FSHOST_H

#include <QThread>
#include <switch.h>

class FSHost : public QThread
{
Q_OBJECT
public:
    explicit FSHost(QObject *parent = 0);
    switch_status_t sendCmd(const char *cmd, const char *args, QString *res);

protected:
    void run(void);

signals:
    void ready(void);

};

#endif // FSHOST_H
