#ifndef ESLEVENT_H
#define ESLEVENT_H

#include <QtGui>
#include "esl_event.h"

class ESLevent
{
private:
    QString _body;
    int _event_id;
    QString _event_name;
public:
    ESLevent();
    ESLevent(esl_event_t *e);
    QString getBody() { return _body; }
    int getID() { return _event_id; }
    QString getType() { return _event_name; }

    QHash<QString, QString> _headers;
};

Q_DECLARE_METATYPE(ESLevent)

#endif // ESLEVENT_H
