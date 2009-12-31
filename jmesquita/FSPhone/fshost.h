#ifndef FSHOST_H
#define FSHOST_H

#include <QThread>
#include <switch.h>

class FSHost : public QThread
{
Q_OBJECT
public:
    explicit FSHost(QObject *parent = 0);
    ~FSHost(void);

protected:
    void run(void);

signals:

public slots:

};

#endif // FSHOST_H
