--[[
dir_user_xml_single_table_pgsql.lua - by Leon de Rooij <leon@toyos.nl>
Makes use of freeswitch.Dbh (experimental) which you can find in contrib/ledr/c/mod_lua_dbh

A single table structure is expected containing all user attrs, params and variables:

CREATE TABLE users (
  "domain"       VARCHAR(255) NOT NULL,
  "id"           VARCHAR(255) NOT NULL,
  "mailbox"      VARCHAR(255),
  "cidr"         VARCHAR(255),
  "number-alias" VARCHAR(255),
  "password"     VARCHAR(255),
  "dial-string"  VARCHAR(255),
  "user_context" VARCHAR(255),
  PRIMARY KEY (domain, id)
);

CREATE INDEX "users_mailbox" ON users (mailbox);
CREATE INDEX "users_cidr" ON users (mailbox);
CREATE INDEX "users_number-alias" ON users ("number-alias");

INSERT INTO users (domain, id, mailbox, cidr, number-alias, password, dial-string, user_context)
       VALUES ('test.com', 'testuser', '1234', NULL, NULL, 'topsecret', NULL, NULL);

To make FreeSWITCH do directory searches through a Lua script, add the following to lua.conf.xml:

    <param name="xml-handler-script" value="dir_user_xml_single_table_pgsql.lua"/>
    <param name="xml-handler-bindings" value="directory"/>
]]

local req_domain = params:getHeader("domain")
local req_key    = params:getHeader("key")
local req_user   = params:getHeader("user")

assert (req_domain and req_key and req_user,
  "This example script only supports generating directory xml for a single user !\n")

local dbh = assert(freeswitch.Dbh("dsn","user","pass"))

-- it's probably wise to sanitize input to avoid SQL injections !
local query = string.format("select * from users where domain = '%s' and \"%s\" = '%s'",
  req_domain, req_key, req_user)

assert (dbh:query(query, function(u) -- there will be only 0 or 1 iteration (limit 1)
  XML_STRING =
[[<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<document type="freeswitch/xml">
  <section name="directory">
    <domain name="]] .. u.domain .. [[">
      <user id="]] .. u.id .. [[" mailbox="]] .. u.mailbox .. [[" cidr="]]
           .. u.cidr .. [[" number-alias="]] .. u["number-alias"] .. [[">
        <params>
          <param name="password" value="]] .. u.password .. [["/>
        </params>
        <variables>
          <variable name="dial-string" value="]] .. u["dial-string"] .. [["/>
          <variable name="user_context" value="]] .. u.user_context .. [["/>
        </variables>
      </user>
    </domain>
  </section>
</document>]]

  -- remove the following line for production
  freeswitch.consoleLog("notice", "Debug from dir_user_xml_single_table_pgsql.lua, generated XML:\n" .. XML_STRING)
end))

--[[ When searched (user_exists id testuser test.com), should return:

<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<document type="freeswitch/xml">
  <section name="directory">
    <domain name="test.com">
      <user id="testuser" mailbox="1234" cidr="" number-alias="">
        <params>
          <param name="password" value="topsecret"/>
        </params>
        <variables>
          <variable name="dial-string" value=""/>
          <variable name="user_context" value=""/>
        </variables>
      </user>
    </domain>
  </section>
</document>

]]
