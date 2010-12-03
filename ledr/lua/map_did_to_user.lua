--[[
map_did_to_user.lua - by Leon de Rooij <leon@toyos.nl>
Looks up did in database and sets a channel variable - this essentially makes mod_odbc_query redundant
Makes use of freeswitch_hdb (experimental) which you can find in contrib/ledr/c/mod_lua_hdb

CREATE TABLE did_users (
  did  VARCHAR(255) NOT NULL,
  user VARCHAR(255) NOT NULL,
  PRIMARY KEY (did),
  KEY did_users_user (user)
);

INSERT INTO users (did, user) VALUES ("1234", "someuser");
]]

local did = argv[1]

local dbh = assert(freeswitch.Dbh("dsn", "user", "pass"))

local my_query = "select user from did_users where did = " .. did

-- set variable - or print to console if no session is available
local function sv(key, val)
  if session then
    session:setVariable(key, val)
  else
    stream:write(string.format("%25s : %s\n", key, val))
  end
end

assert(dbh:query(my_query, function(row)
  for key, val in pairs(row) do     -- in this example only one row with one column will be returned
    sv(key, val)                    -- so here key = 'user'
  end
end))

-- or just set the single value like this:
-- dbh:query(query, function(r) session.setVariable("user", r.user) end)

--[[ You can call it from the dialplan, like this:

<extension name="map_did_to_user" continue="true">
  <condition field="destination_number" expression="^(\+|00)?(\d+)$">
    <action appliction="lua" data="map_did_to_user.lua $2" inline="true"/>
  </condition>
</extension>

<extension name="if_user_then_">
  <condition field="${user}" expression="....

When found, a channel variable named user will be set with the value "someuser"
(you can only use the channel variable in the next condition when using inline, otherwise a transfer is needed)

]]
