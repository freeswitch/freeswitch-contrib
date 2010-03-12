#ifndef PLUGCOMMON_H
#define PLUGCOMMON_H


#define XCDR_PLUGIN_NAME_LEN_MAX 64
#define XCDR_PLUGIN_FILENAME_LEN_MAX 256
#define XCDR_PLUGIN_STRMAXLEN 256


#define PLUG_STATUS_OK 0
//#define PLUG_STATUS_STOPPROCESSING_HERE 1
#define PLUG_STATUS_ERROR -1


typedef struct xcdr_plugin_state
{
   /* local_vars are initialized by plugin during plugin_init
       and destroyed in plugin_free
    */
    void *local_vars;
    
} XCDR_PLUGSTATE, *PXCDR_PLUGSTATE;


#endif
