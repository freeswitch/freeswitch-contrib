#ifndef PLUGLDR_H
#define PLUGLDR_H

#include <pcre.h>
#include "plug.h"


#define XCDR_PLUGLDR_PREPROCESSOR_PLUGIN_TOKEN "preprocessor_plugin"
#define XCDR_PLUGLDR_PROCESSOR_PLUGIN_TOKEN "processor_plugin"
#define XCDR_PLUGLDR_FALLBACK_PLUGIN_TOKEN "fallback_plugin"
#define XCDR_PLUGLDR_PLUGIN_DIR_TOKEN "plugin_dir"

#define XCDR_METRIC_ID_MAX 256
#define XCDR_METRIC_REGEXP_MAX 256



typedef struct xcdr_plugin_metric_tag
{
    char metric [XCDR_METRIC_ID_MAX];
    char regexp [XCDR_METRIC_REGEXP_MAX];
} XCDR_METRTAG, *PXCDR_METRTAG;


typedef struct xcdr_metric_table
{
	XCDR_METRTAG *tags;
	unsigned int tag_count;
} XCDR_METRTAB, *PXCDR_METRTAB;




typedef struct xcdr_plugin_tag
{

    unsigned int id;

    /*plugin priority in config table*/
	unsigned int priority;

    /*plugin file name*/
	char fname [XCDR_PLUGIN_STRMAXLEN];

    /*options passed to plugin*/
	char options [XCDR_PLUGIN_STRMAXLEN];


    /* plugin function pointers*/
	XCDR_PLUGINFO plug;
    
    /*table of plugin metrics*/
    XCDR_METRTAB metrics;

    
} XCDR_PLUGTAG, *PXCDR_PLUGTAG;


typedef struct plugin_table
{
	XCDR_PLUGTAG *tags;
	unsigned int tag_count;
} XCDR_PLUGTAB, *PXCDR_PLUGTAB;


/*First configure PLUGTAB table*/
int xcdr_plugldr_load_config(XCDR_PLUGTAB *plugtab, const char* config_file);

/*Load plugins after successful configuration*/
int xcdr_plugldr_load_plugins (XCDR_PLUGTAB *plugtab);


/*int xcdr_plugldr_match_plugin (const PLUGINCALLINFO *plugci,
                          const XCDR_PLUGTAB *plugtab, XCDR_PLUGINFO **plug,
                          const int startpos);
*/

void xcdr_plugldr_free_plugins (XCDR_PLUGTAB *plugtab);

int xcdr_plug_ldr_sort_plugins (XCDR_PLUGTAB *plugtab);

int xcdr_plugldr_match_regexp (const char *regexp,
                          const char *text, int text_len);


#endif

