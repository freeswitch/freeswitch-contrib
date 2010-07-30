/*
  plugin_init() accepts single option - absolute name of configuration file to parse.
*/

#include "plugcommon.h"
#include "plug.h"
#include "stmtexp.h"
#include "log.h"
#include <syslog.h>
#include <freeradius-client.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include <glib.h>


#define PLUGIN_NAME "rad cdr"
#define PLUGIN_DESC "Rad cdr logger"


/*
#define	PW_FS_PEC		27880

#define PW_FS_AVPAIR		1
#define PW_FS_CLID		2
#define PW_FS_DIALPLAN		3
#define PW_FS_SRC		4
#define PW_FS_DST		5
#define PW_FS_SRC_CHANNEL	6
#define PW_FS_DST_CHANNEL	7
#define PW_FS_ANI		8
#define PW_FS_ANIII		9
#define PW_FS_LASTAPP		10
#define PW_FS_LASTDATA		11
#define PW_FS_DISPOSITION	12
#define PW_FS_HANGUPCAUSE	13
#define PW_FS_BILLUSEC		15
#define PW_FS_AMAFLAGS		16
#define PW_FS_RDNIS		17
#define PW_FS_CONTEXT		18
#define PW_FS_SOURCE		19
#define PW_FS_CALLSTARTDATE	20
#define PW_FS_CALLANSWERDATE	21
#define PW_FS_CALLTRANSFERDATE	22
#define PW_FS_CALLENDDATE	23
*/

/* Statement expansion */

#define VARID_LEN  512

//#define TEMP_GSTRING_INIT_SIZE 1024
//#define STMT_TEMPLATE_MAX_LEN 40960

#define SERVER_MAX 8
#define VSASLEN_MAX 128


#define VSATYPE_STRING "string"
#define VSATYPE_INTEGER "integer"

typedef enum rad_vsa_type
{
    radvsa_string = 0,
    radvsa_integer = 1
} RADVSATYPE;

typedef struct rad_vsas
{
    /* var_id  variable name for lookup in STMTEXP_TAB */
	char var_name [VSASLEN_MAX];
    char vsa_name [VSASLEN_MAX];
	int vsa_id;
	int vsa_pec;
    int vsa_type;
} RADVSAS;



typedef struct radcdr_local_vars
{
    unsigned int sql_insert_retry_max;

    char my_dictionary [XCDR_PLUGIN_STRMAXLEN];
    char my_seqfile [XCDR_PLUGIN_STRMAXLEN];
    char my_deadtime [XCDR_PLUGIN_STRMAXLEN];  /* 0 */
    char my_timeout [XCDR_PLUGIN_STRMAXLEN];	  /* 5 */
    char my_retries [XCDR_PLUGIN_STRMAXLEN];	  /* 3 */
    char my_servers [SERVER_MAX][XCDR_PLUGIN_STRMAXLEN];
    char cf [XCDR_PLUGIN_STRMAXLEN];// = "mod_radius_cdr.conf";

    RADVSAS *vsas;
    int vsas_count;

    rc_handle *rad_config;
} RADCDRVARS;


/*Local variables*/
RADCDRVARS *vars;


char * plugin_get_name ()
{

  return PLUGIN_NAME;
}



char * plugin_get_description ()
{

  return PLUGIN_DESC;
}


/* returns 0 if ok, -1 configuration fails*/
int plugin_config (const char* config_file)
{

    int i;
    config_t cfg;

    memset (vars->my_dictionary, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->my_seqfile, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->my_deadtime, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->my_timeout, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->my_retries, 0x0, XCDR_PLUGIN_STRMAXLEN);
    memset (vars->cf, 0x0, XCDR_PLUGIN_STRMAXLEN);
    for(i = 0; i < SERVER_MAX; i++)
    {
        memset (vars->my_servers[i] , 0x0, XCDR_PLUGIN_STRMAXLEN);
    }

    config_init(&cfg);
    
    if (!config_read_file(&cfg, config_file) )
    {
        fprintf (stderr, "%s plugin_config. Error parsing: '%s'\n",
                 PLUGIN_NAME, config_file);

        config_destroy(&cfg);

        return -1;
    }


    const char *t = NULL;

    if (CONFIG_TRUE == config_lookup_string (&cfg, "config_file", &t))
        snprintf (vars->cf, XCDR_PLUGIN_STRMAXLEN, "%s", t);

    if (CONFIG_TRUE == config_lookup_string (&cfg, "dictionary", &t) )
         snprintf (vars->my_dictionary, XCDR_PLUGIN_STRMAXLEN, "%s", t);
    else strcpy (vars->my_dictionary, "/etc/xmlcdrd/radius/dictionary");

    if (CONFIG_TRUE == config_lookup_string (&cfg, "seq_file", &t) )
        snprintf (vars->my_seqfile, XCDR_PLUGIN_STRMAXLEN, "%s", t);
    else strcpy (vars->my_seqfile, "/tmp/radcdr_seq");
              
    if (CONFIG_TRUE == config_lookup_string (&cfg, "deadtime", &t))
        snprintf (vars->my_deadtime, XCDR_PLUGIN_STRMAXLEN, "%s", t);
    strcpy (vars->my_deadtime, "0");
              
    if (CONFIG_TRUE == config_lookup_string (&cfg, "timeout", &t))
        snprintf (vars->my_timeout, XCDR_PLUGIN_STRMAXLEN, "%s", t);
    strcpy (vars->my_timeout, "5");
  
    if (CONFIG_TRUE == config_lookup_string (&cfg, "retries", &t))
        snprintf (vars->my_retries, XCDR_PLUGIN_STRMAXLEN, "%s", t);
    strcpy (vars->my_retries, "3");

    config_setting_t *setting = config_lookup(&cfg, "servers");
    if(NULL != setting)
    {
        int count = config_setting_length(setting);
        if (SERVER_MAX < count)
            count = SERVER_MAX;

        syslog(LOG_DEBUG, "radcdr %d configured servers:\n", count);

        for(i = 0; i < count; i++)
        {
            config_setting_t *srv = config_setting_get_elem(setting, i);
            
            const char *hostname;
            
            if(!(config_setting_lookup_string(srv, "host", &hostname)))
                continue;
            
            strncpy (vars->my_servers[i], hostname, XCDR_PLUGIN_STRMAXLEN);
            syslog(LOG_DEBUG, "radcdr %d servers: '%s'\n", i, vars->my_servers[i]);
        } //~for
    }


    setting = config_lookup(&cfg, "vsas");
    if(NULL != setting)
    {
        vars->vsas_count = config_setting_length(setting);
        vars->vsas = calloc (vars->vsas_count, sizeof (RADVSAS));

        if (NULL != vars->vsas)
        {
            syslog(LOG_DEBUG, "radcdr: vsas count: %d\n", vars->vsas_count);

            for(i = 0; i < vars->vsas_count; i++)
            {
                config_setting_t *vsas = config_setting_get_elem(setting, i);
            
                const char *var_name;
                const char *vsa_name;
                const char *vsa_type;
                long vsa_id;
                long vsa_pec;
            
                if(!(config_setting_lookup_string(vsas, "var_name", &var_name) &&
                     config_setting_lookup_string(vsas, "vsa_name", &vsa_name) &&
                     config_setting_lookup_int (vsas, "vsa_id", &vsa_id) &&
                     config_setting_lookup_int (vsas, "vsa_pec", &vsa_pec)&&
                     config_setting_lookup_string (vsas, "vsa_type", &vsa_type)
                       ))
                    continue;
            
                strncpy (vars->vsas[i].var_name, var_name, VSASLEN_MAX);
                strncpy (vars->vsas[i].vsa_name, vsa_name, VSASLEN_MAX);
                vars->vsas[i].vsa_id = vsa_id;
                vars->vsas[i].vsa_pec = vsa_pec;
                if (0 == strncasecmp (vsa_type, VSATYPE_INTEGER, strlen(VSATYPE_INTEGER)))
                    vars->vsas[i].vsa_type = radvsa_integer;
                else
                    vars->vsas[i].vsa_type = radvsa_string;
                
            
                syslog(LOG_DEBUG, "radcdr vsa '%s':'%s'\n", 
                       vars->vsas[i].vsa_name, vars->vsas[i].var_name);
            } //~for
        } else
        {
            vars->vsas = NULL;
            syslog(LOG_ERR, "radcdr calloc error\n");
        }
    }


    config_destroy(&cfg);

    syslog (LOG_DEBUG,
             "%s plugin_config: \
 config(%s), dict(%s), seq_file(%s), deadtime(%s), timeout(%s), retries(%s)\n", 
             PLUGIN_NAME,
             vars->cf, vars->my_dictionary,
             vars->my_seqfile,
             vars->my_deadtime,
             vars->my_timeout,
             vars->my_retries);

    return 0;
}



rc_handle *radius_init(void)
{
	int i = 0;
//	rc_handle *rad_config;

	vars->rad_config = rc_new();

	if (NULL == vars->rad_config)
    {
		syslog (LOG_DEBUG, "[mod_radius_cdr] Error initializing rc_handle!\n");
		return NULL;
	}

	vars->rad_config = rc_config_init(vars->rad_config);

	if (NULL == vars->rad_config)
    {
		syslog (LOG_DEBUG, "error initializing radius config!\n");
		rc_destroy(vars->rad_config);
		return NULL;
	}

	/* Some hardcoded ( for now ) defaults needed to initialize radius */
	if (0 != rc_add_config(vars->rad_config, 
                           "auth_order",
                           "radius",
                           "config",
                           0))
    {
		syslog (LOG_DEBUG, "setting auth_order = radius failed\n");
		rc_destroy(vars->rad_config);
		return NULL;
	}

	if (0 != rc_add_config(vars->rad_config,
                           "seqfile",
                           vars->my_seqfile,
                           "config",
                           0))
    {
		syslog (LOG_DEBUG, "setting seqfile = %s failed\n", vars->my_seqfile);
		rc_destroy(vars->rad_config);
		return NULL;
	}


	/* Add the module configs to initialize rad_config */

	for (i = 0; i < SERVER_MAX && vars->my_servers[i][0] != '\0'; i++)
    {
		if (0 != rc_add_config(vars->rad_config,
                          "acctserver", 
                          vars->my_servers[i],
                          vars->cf, 0))
        {
			syslog (LOG_DEBUG,
                    "setting acctserver = %s failed\n",
                    vars->my_servers[i]);
            
			rc_destroy(vars->rad_config);
			return NULL;
		}
        syslog (LOG_DEBUG, "configuring acctserver = %s\n", vars->my_servers[i]);
	}

	if (0 != rc_add_config(vars->rad_config,
                           "dictionary",
                           vars->my_dictionary,
                           vars->cf, 0))
    {
		syslog (LOG_DEBUG, 
                "failed setting dictionary = %s failed\n", 
                vars->my_dictionary);
        
		rc_destroy(vars->rad_config);
		return NULL;
	}
    
	if (0 != rc_add_config(vars->rad_config,
                           "radius_deadtime",
                           vars->my_deadtime,
                           vars->cf, 0))
    {
		syslog (LOG_DEBUG,
                "setting radius_deadtime = %s failed\n",
                vars->my_deadtime);
        
		rc_destroy(vars->rad_config);

		return NULL;
	}


	if (0 != rc_add_config(vars->rad_config,
                           "radius_timeout",
                           vars->my_timeout,
                           vars->cf, 0)) 
    {
		syslog (LOG_DEBUG,
                "setting radius_timeout = %s failed\n",
                vars->my_timeout);
        
		rc_destroy(vars->rad_config);
		return NULL;
	}

	if (0 != rc_add_config(vars->rad_config,
                           "radius_retries",
                           vars->my_retries,
                           vars->cf,
                           0))
    {
		syslog (LOG_DEBUG, "setting radius_retries = %s failed\n", vars->my_retries);
		rc_destroy(vars->rad_config);
		return NULL;
	}

	/* Read the dictionary file(s) */
	if (0 != rc_read_dictionary(vars->rad_config,
                                rc_conf_str(vars->rad_config,
                                            "dictionary")))
    {
		syslog (LOG_DEBUG,
                "reading dictionary file(s): %s\n",
                vars->my_dictionary);
        
		rc_destroy(vars->rad_config);
		return NULL;
	}

	return vars->rad_config;
}


 /**/ 
int plugin_init (const char* filename, const char *options, XCDR_PLUGSTATE *state)
{

    vars = calloc (1, sizeof(RADCDRVARS) );
    state->local_vars = vars;


    if (NULL == vars)
    {
        fprintf (stderr,
                 "%s plugin_init(). Can't allocate memory for state vars\n",
                 PLUGIN_NAME);

        return PLUG_STATUS_ERROR;
    }
    
    plugin_config (options);

    fprintf (stderr, "%s plugin_init (%s) \n", PLUGIN_NAME, options);

    return PLUG_STATUS_OK;
}



int plugin_free (XCDR_PLUGSTATE *state)
{
    vars = state->local_vars;


    free (vars->vsas);
    free (vars);

    
    return PLUG_STATUS_OK;
}



int plugin_main (STMTEXP_TAB *vex, XCDR_PLUGSTATE *state)
{



	VALUE_PAIR *send = NULL;
	uint32_t client_port = 0;
	uint32_t framed_addr = 0;
	uint32_t status_type = PW_STATUS_STOP;


	uint32_t billsec = 0;
	const char *uuid_str = NULL;
    const char *t = NULL;

    int retval;

	char buffer[32] = "";


    vars = state->local_vars;

    int stmt_done = 0;


    uuid_str = stmtexp_lookup_var (vex, "uuid");
    if (NULL != uuid_str)
        syslog (LOG_DEBUG, "radcdr. uuid:'%s'.\n", uuid_str);
    
    /* Create the radius packet */
    vars->rad_config = radius_init();
    
	if (vars->rad_config == NULL)
    {
		syslog (LOG_DEBUG, "radcdr. Error initializing radius, session not logged.\n");
		return PLUG_STATUS_ERROR;
	}

    
	/* Set Status Type */
	if (rc_avpair_add(vars->rad_config, &send, PW_ACCT_STATUS_TYPE, &status_type, -1, 0) == NULL)
    {
		syslog (LOG_DEBUG, "failed adding  PW_ACCT_STATUS_TYPE \n");
		rc_destroy(vars->rad_config);
		//goto end;
	}

	if (rc_avpair_add(vars->rad_config, &send, PW_ACCT_SESSION_ID, (void*)uuid_str, -1, 0) == NULL)
    {
		syslog (LOG_DEBUG, "failed adding Acct-Session-ID: %s\n", uuid_str);
		rc_destroy(vars->rad_config);
		//goto end;
	}

    /* Add custom VSA's from config */

    syslog (LOG_DEBUG, "rad custom vsas configured '%d'\n", vars->vsas_count);

    int i;
    for (i = 0; i < vars->vsas_count; i++)
    {
        syslog (LOG_DEBUG, "rad custom vsa, var lookup '%s'\n", vars->vsas[i].var_name);
        t = stmtexp_lookup_var (vex, vars->vsas[i].var_name);
        
        if (NULL != t)
        {
            
            syslog (LOG_DEBUG, 
                    "radcdr adding rad vsa '%s': '%s' pec:%d\n",
                    vars->vsas[i].vsa_name,
                    t, 
                    vars->vsas[i].vsa_pec);
        
            int res = 0;
        
            if (radvsa_string == vars->vsas[i].vsa_type)
            {
            
                if (NULL != rc_avpair_add(vars->rad_config, 
                                     &send, 
                                     vars->vsas[i].vsa_id,
                                     (void *) t,
                                     -1,
                                          vars->vsas[i].vsa_pec))
                    res = 1;
            
            }
            else
            {
                int integer = atoi (t);
            
                if (NULL != rc_avpair_add(vars->rad_config,
                                     &send, 
                                     vars->vsas[i].vsa_id,
                                     &integer,
                                     -1,
                                          vars->vsas[i].vsa_pec))
                    res = 1;
            }

            if (0 == res)
            {
                syslog (LOG_DEBUG, "failed adding vsa %s\n", vars->vsas[i].vsa_name);
            
                //rc_destroy(vars->rad_config);
                break;
            }
        
        } else syslog (LOG_DEBUG, "lookup of vsa var %s failed\n", vars->vsas[i].var_name);
        
    }
    
    

	if (rc_acct(vars->rad_config, client_port, send) == OK_RC) {
		syslog(LOG_DEBUG, "radcdr RADIUS Accounting OK\n");
		retval = PLUG_STATUS_OK;
	} else {
		syslog(LOG_DEBUG, "radcdr RADIUS Accounting Failed\n");
		retval = PLUG_STATUS_ERROR;
	}

	rc_avpair_free(send);
	rc_destroy(vars->rad_config);



    return retval;
}
