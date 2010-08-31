local env = assert(luasql.scdb()) 
local con = assert(env:connect("dsn","user","pass")) 
local res = assert(con:exec2table("select 1 as foo, 2 as bar")) 
 
for i = 1, table.getn(res) do 
  for key, val in pairs(res[i]) do 
    stream:write(string.format("%25s : %s\n", key, val)) 
  end 
  stream:write("\n") 
end 
 
con:close(); 
env:close();
