--[[
map_did_to_user.lua - by Leon de Rooij <leon@toyos.nl>
Looks up did in database and sets a channel variable - this essentially makes mod_odbc_query redundant
Makes use of luasql.scdb (experimental) which you can find in contrib/ledr/c/mod_lua_luasql

CREATE TABLE did_users (
  did  VARCHAR(255) NOT NULL,
  user VARCHAR(255) NOT NULL,
  PRIMARY KEY (did),
  KEY did_users_user (user)
);

INSERT INTO users (did, user) VALUES ("1234", "someuser");
]]

local env = assert(luasql.scdb()) 
local con = assert(env:connect("dsn","user","pass")) 

local did = argv[1]

local query = "select user from did_users where did = " .. did

local res = assert(con:exec2table(query)) 

if table.getn(res) == 1 then
  session.setVariable("user", res[1]["user"])
end

--[[ You can call it from the dialplan, like this:

<extension name="map_did_to_user">
  <condition field="destination_number" expression="^(\+|00)?(\d+)$">
    <action appliction="lua" data="map_did_to_user.lua $2" inline="true"/>
  </condition>
</extension>

<extension name="if_user_then_">
  <condition field="${user}" expression="....

When found, a channel variable named user will be set with the value "1234"
(you can only use the channel variable in the next condition when using inline, otherwise a transfer is needed)

]]
