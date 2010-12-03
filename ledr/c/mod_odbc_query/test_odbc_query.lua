-- copy this file to scripts/ in your fs install dir
-- and execute from the cli as 'luarun test_odbc_query.lua'
-- you should get an output comparable to: [NOTICE] switch_cpp.cpp:1177 foo = [1]

api = freeswitch.API();

local r = {}

setfenv (
  assert (
    loadstring (
      api:execute( "odbc_query", "lua select 1 as foo" )
    )
  ), r
) ()

freeswitch.consoleLog( "notice", "foo = [" .. r.result[1].foo .. "]\n" )

