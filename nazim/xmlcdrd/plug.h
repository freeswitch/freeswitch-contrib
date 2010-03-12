#ifndef PLUG_H
#define PLUG_H

#include "plugcommon.h"
#include "stmtexptab.h"


typedef struct xcdr_plugin_info
{
	void *handle;
	char name [XCDR_PLUGIN_NAME_LEN_MAX];

    XCDR_PLUGSTATE state;

    /*Functions exported by plugin*/

    int (*plugin_init) (const char *filename, const char *options,
                        XCDR_PLUGSTATE *state);

    int (*plugin_free) (XCDR_PLUGSTATE *state);

    char* (*plugin_get_description) (void);
    char* (*plugin_get_name) (void);

    /*
      Main function which processes a hash table of variables/values @vex
      Function returns:
      PLUG_STATUS_OK
      PLUG_STATUS_ERR */

    int (*plugin_main) (STMTEXP_TAB *vex, XCDR_PLUGSTATE *state);

    /* ~Functions exported by plugin*/

} XCDR_PLUGINFO, *PXCDR_PLUGINFO;

/* plug_int() calls plugin_init() which initializes
   XCDR_PLUGSTATE.state used by plugin internally */
int plug_init (const char *filename, const char *options, XCDR_PLUGINFO *plug);

/* plug_free() calls plugin_free() to deallocate all dynamic resources
   bound to XCDR_PLUGSTATE.local_vars. Must be invoked before plug_destroy() */
int plug_free (XCDR_PLUGINFO *plug);

/* plug_destroy() calls library dlclose() AFTER plug_free() */
int plug_destroy(XCDR_PLUGINFO *plug);

#endif
