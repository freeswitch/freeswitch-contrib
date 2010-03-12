--  Lua script _must_ provide three functions:
--  luaexec_init()
--  luaexec_free()
--  luaexec_man()
--  Functions listed above can be empty, but they must exist for successful run.


require "luasql.odbc"

env = nil
db_conn = nil
count = 0

function luaexec_init ()
  env = assert(luasql.odbc())
  db_conn = assert(env:connect("ddp") )
  count = 1
end


function luaexec_free ()
  db_conn:close()
  env:close()
end

function luaexec_main ()

  uuid = xcdr_varget("uuid")
  billsec = xcdr_varget("billsec")

  db_conn:execute(string.format("insert into planeta_cdr_notify (uuid,billsec) values ('%s', %d)", uuid, billsec + count))
  count = count + 1
end

