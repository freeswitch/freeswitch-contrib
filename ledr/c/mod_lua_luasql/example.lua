local env = assert(luasql.scdb()) -- same as regular luasql
local con = assert(env:connect("dsn","user","pass")) -- same as regular luasql

local res = assert(con:exec2table("select 1 as foo, 2 as bar")) -- all your rows in a single table

--[[ so, now res will look like this :
{
  [1] = {
    ["foo"] = "1";
    ["bar"] = "2";
  };
}
]]
 
for i = 1, table.getn(res) do -- loop through rows
  for key, val in pairs(res[i]) do -- loop through columns
    stream:write(string.format("%25s : %s\n", key, val)) 
  end 
  stream:write("\n") 
end 
 
con:close(); -- optional, it is also closed at first gc run,
 -- although it does release the dbh for re-use by another thread so can be efficient to do it early

env:close(); -- optional, (same reason) just don't close your env before closing the con !
