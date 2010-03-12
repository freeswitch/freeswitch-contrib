/*
  plugin_init() accepts single option - absolute name of configuration file to parse.
*/
#include "plugcommon.h"
#include "plug.h"
#include "stmtexp.h"
#include "log.h"
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libconfig.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <dlfcn.h>

#define PLUGIN_NAME "lua exec"
#define PLUGIN_DESC "Lua script wrapper"

#define FILENAME_LEN_MAX 512
#define STR_LEN_MAX 256

/*that's what lua script would call to get variable*/
#define LUA_VAR_GET_FUNC "xcdr_varget"

typedef struct luaexec_local_vars
{
    char *t;

    int cfgres;
    config_t cfg;
    config_setting_t *setting;


    luaL_reg *lualibs;
    int lualibs_count;

    char luascript [FILENAME_LEN_MAX];
    lua_State *L;

    STMTEXP_TAB *vex;

} LUAEXECVARS;


/*Local variables*/
LUAEXECVARS *vars;



char * plugin_get_name ()
{

  return PLUGIN_NAME;
}



char * plugin_get_description ()
{

  return PLUGIN_DESC;
}


static int l_varget (lua_State *L)
{
    const char* varid = luaL_checkstring(L, 1);

    const char* val = stmtexp_lookup_var (vars->vex, varid);

    lua_pushstring(L, val);

    return 1;
}


 /**/ 
int plugin_init (const char* filename, const char *options, XCDR_PLUGSTATE *state)
{

    int i;

    vars = calloc (1, sizeof(LUAEXECVARS) );
    state->local_vars = vars;


    if (NULL == vars)
    {
        fprintf (stderr,
                 "%s plugin_init(). Can't allocate memory for state vars\n",
                 PLUGIN_NAME);
        return PLUG_STATUS_ERROR;
    }
    
    fprintf (stderr, "%s plugin_init (%s) \n", PLUGIN_NAME, options);

    config_init(&vars->cfg);
    
    if (!config_read_file(&vars->cfg, options) )
    {
        fprintf (stderr, "%s plugin_init (%s). Error parsing: '%s'\n",
                 PLUGIN_NAME, options, options);

        config_destroy(&vars->cfg);

        return PLUG_STATUS_ERROR;
    }


    memset (vars->luascript , 0x0, FILENAME_LEN_MAX);

    const char *t = NULL;
    
    if (CONFIG_TRUE == config_lookup_string (&vars->cfg, "luascript", &t) )
        snprintf (vars->luascript, FILENAME_LEN_MAX - 1, "%s", t);

    config_destroy(&vars->cfg);
    return PLUG_STATUS_OK;
}


int plugin_free (XCDR_PLUGSTATE *state)
{
    vars = state->local_vars;

    free (vars);
    
    return PLUG_STATUS_OK;
}


int plugin_main (STMTEXP_TAB *vex, XCDR_PLUGSTATE *state)
{

    vars = state->local_vars;
    vars->vex = vex;

    vars->L = lua_open();
    luaL_openlibs (vars->L);

    lua_pushcfunction(vars->L, l_varget);
    lua_setglobal(vars->L, LUA_VAR_GET_FUNC);

    
    if (luaL_loadfile(vars->L, vars->luascript) )
        fprintf(stderr, "can't load file: %s, %s\n", vars->luascript,
            lua_tostring(vars->L, -1));
    

    if(lua_pcall(vars->L, 0, 0, 0) )
        fprintf(stderr, "can't run file: %s, %s\n", vars->luascript,
                lua_tostring(vars->L, -1));
    
    lua_close (vars->L);
    
    return PLUG_STATUS_OK;
}
