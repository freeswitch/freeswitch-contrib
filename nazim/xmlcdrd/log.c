#include "log.h"
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void syslog_open (const char *ident, int facility) {
	openlog(ident, LOG_PID, facility);
	return;
}

void syslog_close () {
	closelog ();
	return;
}

void syslog_error (const char *message) {
	syslog (LOG_ALERT, "error. %s", message);
}

void syslog_msg (int priority, int echotoconsole, const char *fmt, ...) {
	int n, size;
	char *p;
	va_list ap;
	
	size = 100;
	if (NULL == (p = malloc (size))) {
		fprintf (stderr, "syslog_msg failed. malloc error %s:%d\r\n", __FILE__, __LINE__);
		return;
	}

	while (1) {
		va_start(ap, fmt);
		n = vsnprintf (p, size, fmt, ap);
		va_end(ap);

		if (n > -1 && n < size) {
			syslog (priority, "%s", p);
			if (echotoconsole) fprintf (stderr, "%s\r\n", p);
			free (p);
			return;
		}
      
		if (n > -1) size = n+1;
		else size *= 2;
		
		if ((p = realloc (p, size)) == NULL) {
			fprintf (stderr, "syslog_msg failed. realloc error %s:%d\r\n", __FILE__, __LINE__);
			return;
		}
   	}
}

