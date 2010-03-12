/*
  plugin_init() accepts single option - absolute name of configuration file to parse.
*/

#include "plugcommon.h"
#include "plug.h"
#include "stmtexp.h"
#include "log.h"
#include <syslog.h>
#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include <glib.h>


#define PLUGIN_NAME "mysql cdr"
#define PLUGIN_DESC "MySQL cdr logger"

#define STMT_RETRY_MAX   2
#define MYSQL_IPPORT 3306




/* Statement expansion */
#define VARSTART "$<"
#define VAREND ">"

#define VARSTART_LEN strlen (VARSTART)
#define VAREND_LEN strlen (VAREND)

#define VARID_LEN  512

#define TEMP_GSTRING_INIT_SIZE 1024
#define STMT_TEMPLATE_MAX_LEN 40960

typedef struct mysqlcdr_local_vars
{
    unsigned int sql_insert_retry_max;

    GString *temp;
    
    MYSQL *conn; 
    char dbuserid [XCDR_PLUGIN_STRMAXLEN];
    char dbpasswd [XCDR_PLUGIN_STRMAXLEN];
    char dbschema [XCDR_PLUGIN_STRMAXLEN];
    char dbipaddr [XCDR_PLUGIN_STRMAXLEN];
    unsigned int  dbipport;
    unsigned long dbflags;
    char stmt_template [STMT_TEMPLATE_MAX_LEN];

    char *t;

    int cfgres;
    config_t cfg;

} MYSQLCDRVARS;


/*Local variables*/
MYSQLCDRVARS *vars;


int mysql_query_retry_on_fail (MYSQL *conn, const char *stmt,
                               unsigned int retrycount)
{

    unsigned int rc = 0;
    int res;

    while (rc++ <= retrycount)
    {
        res = mysql_query (conn, stmt);
        if (0 != res)
        {
            mysql_print_error (conn, "mysql_query() failed.");

            if (-1 == mysql_ping(conn) )
                fprintf (stderr, "Connection to db lost\n");

        } else return res;

    }

}




/*Returns NULL if escaping of string fails*/
const char *mysqlcdr_expand_str (MYSQL *conn, stmtexp_tab_t *vex, 
                         const char *src, int srclen)
{
    const char *s; //src
    char *e; //end
    char varid [VARID_LEN];
    
    s = src;
    g_string_set_size (vex->expanded, 0);

    //fprintf (stderr, "mysqlcdr_expand_str '%s'\n", src);

    while (s < src + srclen)
    {
        if (NULL != (e = strstr (s, VARSTART)) )
        {
            g_string_append_len (vex->expanded, s, e - s);
            s = e++;

            if (NULL != (e = strstr (s, VAREND)) )
            {
                
                /* Remove placeholders VARSTART and VAREND, then lookup */
                memset (varid, 0x0, VARID_LEN);
                strncpy (varid, s + VARSTART_LEN, (e - s) - VARSTART_LEN );
                
                const char* ev = stmtexp_lookup_var (vex, varid);
                //fprintf (stderr, "---%s\n", ev);
                if (NULL != ev)
                {
                    unsigned int ev_len = strlen(ev);

                    g_string_set_size (vars->temp, (ev_len * 2) + 1);
                    memset (vars->temp->str, 0x0, (ev_len * 2) + 1);

                    unsigned long res = mysql_real_escape_string (
                            conn,
                            vars->temp->str,
                            ev,
                            ev_len
                        );

                    /*if length of escaped string is less than input bail out*/
                    if (res < ev_len)
                        return NULL;
                   
                    /*append expanded var to destination*/
                    g_string_append (vex->expanded, vars->temp->str);
                }
                else
                {
                    /*variable was not found, skip it*/
                }
                
                s = e + 1;
                //fprintf (stderr, "s: '%s'\n", s);
         
            }
            else   /*no more signatures found, append the rest of the str*/
            {
                if (NULL != s)
                    g_string_append (vex->expanded, s);
                break;
            }
        }
       else /*no more signatures found, append the rest of the str*/
        {
            if (NULL != s)
                g_string_append (vex->expanded, s);
            break;
        }
    }//while (s < src + src_len)

    return vex->expanded->str;
    
}



char * plugin_get_name ()
{

  return PLUGIN_NAME;
}



char * plugin_get_description ()
{

  return PLUGIN_DESC;
}

 /**/ 
int plugin_init (const char* filename, const char *options, XCDR_PLUGSTATE *state)
{

    vars = calloc (1, sizeof(MYSQLCDRVARS) );
    state->local_vars = vars;


    if (NULL == vars)
    {
        fprintf (stderr,
                 "%s plugin_init(). Can't allocate memory for state vars\n",
                 PLUGIN_NAME);
        return PLUG_STATUS_ERROR;
    }
    
    fprintf (stderr, "%s plugin_init (%s) \n", PLUGIN_NAME, options);


    vars->temp = g_string_sized_new (TEMP_GSTRING_INIT_SIZE);

    config_init(&vars->cfg);
    
    if (!config_read_file(&vars->cfg, options) )
    {
        fprintf (stderr, "%s plugin_init (%s). Error parsing: '%s'\n",
                 PLUGIN_NAME, options, options);

        config_destroy(&vars->cfg);

        return PLUG_STATUS_ERROR;
    }


    memset (vars->dbuserid, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->dbpasswd, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->dbipaddr, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->dbschema, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->stmt_template, 0x0, XCDR_PLUGIN_STRMAXLEN);
    

    const char *t = NULL;
    int it;

    if (CONFIG_TRUE == config_lookup_string (&vars->cfg, "dbuserid", &t) )
         snprintf (vars->dbuserid, XCDR_PLUGIN_STRMAXLEN, "%s", t);

    if (CONFIG_TRUE == config_lookup_string (&vars->cfg, "dbpasswd", &t) )
        snprintf (vars->dbpasswd, XCDR_PLUGIN_STRMAXLEN, "%s", t);
              
    if (CONFIG_TRUE == config_lookup_string (&vars->cfg, "dbipaddr", &t))
        snprintf (vars->dbipaddr, XCDR_PLUGIN_STRMAXLEN, "%s", t);
              
    if (CONFIG_TRUE == config_lookup_string (&vars->cfg, "dbschema", &t))
        snprintf (vars->dbschema, XCDR_PLUGIN_STRMAXLEN, "%s", t);
  
    if (CONFIG_TRUE == config_lookup_string (&vars->cfg, "stmt_template", &t))
        snprintf (vars->stmt_template, STMT_TEMPLATE_MAX_LEN, "%s", t);
              
    if (CONFIG_TRUE != config_lookup_int (&vars->cfg, "dbipport", &vars->dbipport) )
        vars->dbipport = MYSQL_IPPORT;

    if (CONFIG_TRUE != config_lookup_int (&vars->cfg, "sql_insert_retry_max",
                                          &vars->sql_insert_retry_max) )
        vars->sql_insert_retry_max = STMT_RETRY_MAX;

    if (0 >= vars->sql_insert_retry_max)
        vars->sql_insert_retry_max = STMT_RETRY_MAX;


    config_destroy(&vars->cfg);

    fprintf (stderr, "%s plugin_init (%s). \
dbschema: %s, dbuserid: %s, dbpasswd: %s, dbipaddr: %s, \
dbipport: %i \r\n",
             PLUGIN_NAME, options,
             vars->dbschema, vars->dbuserid, vars->dbpasswd, 
             vars->dbipaddr, vars->dbipport);


    vars->dbflags = 0;
    vars->conn = (MYSQL *) mysql_do_connect (vars->dbipaddr, 
                                            vars->dbuserid, vars->dbpasswd,
                                            vars->dbschema,
                                            vars->dbipport, NULL, vars->dbflags);
    if (vars->conn == NULL)
    {
        fprintf (stderr, "%s plugin_init (%s). Can't connect to db: '%s'",
                 PLUGIN_NAME, options, vars->dbschema);

        syslog_msg (LOG_ALERT, 0, "%s plugin_init (%s). Can't connect to db: '%s'\n",
                    PLUGIN_NAME, options, vars->dbschema);

        return PLUG_STATUS_ERROR;
    }
    
    return PLUG_STATUS_OK;
}

int plugin_free (XCDR_PLUGSTATE *state)
{
    vars = state->local_vars;

    mysql_close (vars->conn);
    g_string_free (vars->temp, TRUE);

    free (vars);
    
    return PLUG_STATUS_OK;
}


int plugin_main (STMTEXP_TAB *vex, XCDR_PLUGSTATE *state)
{

    vars = state->local_vars;
   
    int retr = vars->sql_insert_retry_max;
    int stmt_done = 0;

    while (retr--)
    {
        const char* stmt =  mysqlcdr_expand_str (vars->conn, vex, vars->stmt_template,
                                                 strlen(vars->stmt_template));
        if (NULL == stmt)
        {
            fprintf (stderr, "mysqlcdr_expand_str() returned error. Retrying\n");
            mysql_ping(vars->conn);
        }
        else
        {
         
            if (0 != mysql_query_retry_on_fail (vars->conn, stmt, STMT_RETRY_MAX))
                fprintf (stderr, "Error. '%s' failed. Retrying.\n", stmt);
            else
            {
                fprintf (stderr, "%s\n", stmt);
                stmt_done = 1;
                break;
            }
        }
            
    }//~while (retr--)

    if (!stmt_done)
    {
        //!FixMe - Failover action. Log to file ... e.t.c
    }

    return PLUG_STATUS_OK;
}
