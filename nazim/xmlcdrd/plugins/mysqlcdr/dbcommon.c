#include <stdio.h>
#include <syslog.h>
#include "dbcommon.h"
#include "string.h"
#include "log.h"

MYSQL *mysql_do_connect(char *host_name,
                        char *user_name, char *password, char *db_name,
                        unsigned long port_num, char *socket_name,
                        unsigned int flags)
{

    MYSQL *conn; /* pointer to connection handler */

    conn = mysql_init(NULL); /* allocate, initialize connection handler */
    if (conn == NULL)
    {
        //fprintf (stderr, "mysql_init() failed\n");
        syslog_error("mysql_init() failed");
        return (NULL);
    }

    my_bool reconnect = 1;
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);

    if (mysql_real_connect(conn, host_name, user_name, password,
                           db_name, port_num, socket_name, flags) == NULL)
    {
        fprintf (stderr, "mysql_real_connect() failed:\nError %u (%s)\n", mysql_errno (conn), mysql_error (conn));
        //syslog_mysql_error(conn, "error. mysql_real_connect() failed");
        return (NULL);
    }

    return (conn);
}

void mysql_do_disconnect(MYSQL *conn)
{
    mysql_close(conn);
}

void mysql_print_error(MYSQL *conn, char *message)
{

    if (conn != NULL)
    {
        syslog_msg(LOG_ALERT, 1, "MySQL connection error. '%s'. %u (%s)",
                   message, mysql_errno(conn), mysql_error(conn));

        fprintf (stderr, "MySQL connection error. '%s'. %u (%s)",
                   message, mysql_errno(conn), mysql_error(conn));
    }
    else
    {
        fprintf (stderr, "MySQL connection error. \
'%s'. Can't get mysql_errno() because connection handler is invalid", message);

        syslog_msg(LOG_ALERT, 0,
                   "MySQL connection error. \
'%s'. Can't get mysql_errno() because connection handler is invalid", message);
        
    }
}

