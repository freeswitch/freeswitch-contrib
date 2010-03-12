#ifndef LOG_H
#define LOG_H

#include <stdlib.h>

void syslog_open (const char *ident, int facility);
void syslog_close (void);

void syslog_msg (int priority, int echotoconsole, const char *fmt, ...);
void syslog_error (const char *message);

#endif

