#include <QtGui>
#include "eslevent.h"


ESLevent::ESLevent()
{
}

ESLevent::ESLevent(esl_event_t *e)
{
    for (esl_event_header_t *hp = e->headers; hp; hp = hp->next)
    {
        _headers[hp->name] = hp->value;
    }
    _body = e->body;
    _event_id = e->event_id;
    _event_name = esl_event_name(e->event_id);
}
