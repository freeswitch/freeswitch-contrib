/*
** LuaMod: A simplification of the LuaSQL Module / Driver loading mechanism
** LuaSQL Credits: http://www.keplerproject.org/luasql/index.html#credits
*/

#include <switch.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_WIN32)
#include <windows.h>
#endif

#include "lua.h"
#include "lauxlib.h"

#include "luamod.h"


#define DRIVER_PREFIX "Test: "
#define DRIVER_ENV "Driver Environment"
#define DRIVER_OBJ "Driver Object"


typedef struct {
	short closed;
	int obj_counter;
} env_t;


typedef struct {
  short closed;
  int env; /* reference to environment */
  char *mandatory_property;
  char *optional_property;
} obj_t;


LUAMOD_API int luaopen_luamod_test (lua_State *L);


/*
** Check for valid env.
*/
static env_t *get_env (lua_State *L)
{
	env_t *env = (env_t *) luaL_checkudata (L, 1, DRIVER_ENV);
	luaL_argcheck (L, env != NULL, 1, LUAMOD_PREFIX"env expected");
	luaL_argcheck (L, !env->closed, 1, LUAMOD_PREFIX"env is closed");
	return env;
}


/*
** Check for valid obj.
*/
static obj_t *get_obj (lua_State *L)
{
	obj_t *obj = (obj_t *) luaL_checkudata (L, 1, DRIVER_ENV);
	luaL_argcheck (L, obj != NULL, 1, LUAMOD_PREFIX"obj expected");
	luaL_argcheck (L, !obj->closed, 1, LUAMOD_PREFIX"obj is closed");
	return obj;
}


/*
** Pushes true and returns 1
*/
static int pass(lua_State *L)
{
  lua_pushboolean (L, 1);
  return 1;
}


/*
** Closes an object
*/
static int obj_close (lua_State *L)
{
  env_t *env;
  obj_t *obj = (obj_t *) luaL_checkudata (L,1,DRIVER_OBJ);

  luaL_argcheck (L, obj != NULL, 1, LUAMOD_PREFIX"object expected");
  if (obj->closed) {
    lua_pushboolean (L, 0);
    return 1;
  }

  /* Decrement objection counter on environment object */
  lua_rawgeti (L, LUA_REGISTRYINDEX, obj->env);
  env = (env_t *) lua_touserdata (L, -1);
  env->obj_counter--;

  /* Nullify structure fields. */
  obj->closed = 1;
  obj->mandatory_property = NULL; /* perhaps do some free'ing here if necessary */
  obj->optional_property = NULL;

  luaL_unref (L, LUA_REGISTRYINDEX, obj->env);

  return pass(L);
}


static int obj_method (lua_State *L)
{
  obj_t *obj = (obj_t *) get_obj (L);
  if (strcmp(obj->mandatory_property, "the right value")) /* or do something more interesting */
    return luamod_faildirect (L, LUAMOD_PREFIX"your mandatory_property doesn't have the right value");
  return 1; /* ok */
}


/*
** Instantiates a new object
** Lua Input: mandatory [, optional]
**   mandatory: you must supply this argument
**   optional: you may leave this argument out
** Lua Returns:
**   object if successfull
**   nil and error message otherwise.
*/
static int env_inst_obj (lua_State *L)
{
	env_t *env = (env_t *) get_env (L);

	const char *mandatory = luaL_checkstring (L, 2);
	const char *optional = luaL_optstring (L, 3, NULL);

  obj_t *obj = (obj_t *) lua_newuserdata (L, sizeof (obj_t));
  luamod_setmeta (L, DRIVER_OBJ);

  obj->closed = 0;
  obj->env = LUA_NOREF;

  obj->mandatory_property = (char *) mandatory;
  obj->optional_property = (char *) optional;

  lua_pushvalue (L, 1);
  obj->env = luaL_ref (L, LUA_REGISTRYINDEX);

  env->obj_counter++;
	return 1;
}


/*
** Closes an env object - this function is automatically called on 1st gc try
*/
static int env_close (lua_State *L)
{
	env_t *env = (env_t *) luaL_checkudata (L, 1, DRIVER_ENV);
	luaL_argcheck (L, env != NULL, 1, LUAMOD_PREFIX"env expected");
	if (env->closed) {
		lua_pushboolean (L, 0);
		return 1;
	}
	if (env->obj_counter > 0)
		return luaL_error (L, LUAMOD_PREFIX"there are open objects");

	env->closed = 1;

	return pass(L);
}


/*
** Create metatables for each class of object.
*/
static void create_metatables (lua_State *L)
{
	struct luaL_reg env_methods[] = {
		{"close", env_close},
		{"obj", env_inst_obj},
		{NULL, NULL},
	};
	struct luaL_reg obj_methods[] = {
		{"close", obj_close},
		{"method", obj_method},
		{NULL, NULL},
	};
	luamod_createmeta (L, DRIVER_ENV, env_methods);
	luamod_createmeta (L, DRIVER_OBJ, obj_methods);
	lua_pop (L, 2); /* 2 for the amount of metatables created */
}


/*
** Creates an Environment and returns it.
*/
static int create_env (lua_State *L)
{
	env_t *env;
	create_metatables (L);
	env = (env_t *) lua_newuserdata (L, sizeof (env_t));
	luamod_setmeta (L, DRIVER_ENV);
	/* fill in structure */
	env->closed = 0;
	env->obj_counter = 0;
	return 1;
}


/*
** Creates the metatables for the objects and registers the
** driver open method.
*/
LUAMOD_API int luaopen_luamod_test (lua_State *L)
{
	struct luaL_reg driver[] = {
		{"my_env", create_env},
		{NULL, NULL},
	};
	luaL_openlib (L, LUAMOD_TABLENAME, driver, 0);
	luamod_set_info (L);
	return 1;
}
