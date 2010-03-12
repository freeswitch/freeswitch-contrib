#ifndef DBCOMMON_H
#define DBCOMMON_H

#include <mysql.h>


MYSQL *mysql_do_connect (char *host_name,
        char *user_name, char *password, char *db_name,
        unsigned long port_num, char *socket_name, unsigned int flags);


void mysql_do_disconnect (MYSQL *conn);
void mysql_print_error (MYSQL *conn, char *message);

#endif

