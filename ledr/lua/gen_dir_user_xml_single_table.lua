-- gen_dir_user_xml_single_table.lua - by Leon de Rooij <leon@toyos.nl>
-- makes use of luasql.scdb which you can find in contrib/ledr/c/mod_lua_luasql
-- a single table structure is expected containing all user attrs, params and variables
-- it's probably wise to sanitize input to avoid SQL injections !
local query = string.format("select * from users where `domain`='%s' and `%s`='%s' limit 1",
  req_domain, req_key, req_user)

local req_domain = params:getHeader("domain")
local req_key    = params:getHeader("key")
local req_user   = params:getHeader("user")

assert (req_domain and req_key and req_user,
  "This example script only supports generating directory xml for a single user !\n")

local env = assert(luasql.scdb()) 
local con = assert(env:connect("dsn","user","pass")) 

local res = assert(con:exec2table(query)) 

if table.getn(res) == 1 then
  local u = res[1]
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
end
