/*
** LuaMod: A simplification of the LuaSQL Module / Driver loading mechanism
** LuaSQL Credits: http://www.keplerproject.org/luasql/index.html#credits
*/

#ifndef _LUAMOD_
#define _LUAMOD_

#ifndef LUAMOD_API
#define LUAMOD_API
#endif

#define LUAMOD_PREFIX "LuaMod: "
#define LUAMOD_TABLENAME "luamod"

typedef struct {
	short  closed;
} pseudo_data;

LUAMOD_API int luamod_faildirect (lua_State *L, const char *err);
LUAMOD_API int luamod_createmeta (lua_State *L, const char *name, const luaL_reg *methods);
LUAMOD_API void luamod_setmeta (lua_State *L, const char *name);
LUAMOD_API void luamod_set_info (lua_State *L);

#endif
