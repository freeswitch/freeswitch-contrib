--print(xcdr_varget("uuid"), xcdr_varget("billsec"), xcdr_varget("uuid") )

uuid = xcdr_varget("uuid")
billsec = xcdr_varget("billsec")


require "luasql.odbc"
env = assert(luasql.odbc())
db_conn = assert(env:connect("ddp") )
db_conn:execute(string.format("insert into cdr (uuid,billsec) values ('%s', %d)", uuid, billsec))

db_conn:close()
env:close()

--local file = assert(io.open("output.lua", "w"))
--file:write("Hello world\n")
--file:close()

