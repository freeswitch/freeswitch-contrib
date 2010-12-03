--[[

!! THIS EXAMPLE HAS NOT BEEN TESTED YET !!

dir_user_xml_multi_table_mysql.lua - by Leon de Rooij <leon@toyos.nl>
Makes use of luasql.scdb (experimental) which you can find in contrib/ledr/c/mod_lua_luasql

A multi table structure is expected:

CREATE TABLE `dir_domains` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `dir_domains_name` (`name`)
);

CREATE TABLE `dir_domain_params` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_domain_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  FOREIGN KEY (`dir_domain_id`) REFERENCES `dir_domains` (`id`)
);

CREATE TABLE `dir_domain_variables` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_domain_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  FOREIGN KEY (`dir_domain_id`) REFERENCES `dir_domains` (`id`)
);

CREATE TABLE `dir_groups` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`)
);

CREATE TABLE `dir_groups_users` (
  `dir_group_id` INT NOT NULL,
  `dir_user_id` INT NOT NULL,
  FOREIGN KEY (`dir_group_id`) REFERENCES `dir_groups` (`id`),
  FOREIGN KEY (`dir_user_id`) REFERENCES `dir_users` (`id`)
);

CREATE TABLE `dir_users` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_domain_id` INT NOT NULL,
  `enabled` TINYINT(1) NOT NULL,
  `username` VARCHAR(255) NOT NULL,
  `cidr` VARCHAR(255),
  `mailbox` VARCHAR(255),
  `number-alias` VARCHAR(255),
  PRIMARY KEY (`id`),
  KEY `dir_users_username` (`username`),
  KEY `dir_users_cidr` (`cidr`),
  KEY `dir_users_mailbox` (`mailbox`),
  KEY `dir_users_number-alias` (`number-alias`),
  FOREIGN KEY (`dir_domain_id`) REFERENCES `dir_domains` (`id`)
);

CREATE TABLE `dir_user_params` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_user_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY  (`id`),
  FOREIGN KEY (`dir_user_id`) REFERENCES `dir_users` (`id`)
);

CREATE TABLE `dir_user_variables` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `dir_user_id` INT NOT NULL,
  `name` VARCHAR(255) NOT NULL,
  `value` VARCHAR(255) NOT NULL,
  PRIMARY KEY  (`id`),
  FOREIGN KEY (`dir_user_id`) REFERENCES `dir_users` (`id`)
);

INSERT INTO `dir_domains` (`id`, `name`) VALUES (1, 'test.com');
INSERT INTO `dir_domain_params` (`id`, `dir_domain_id`, `name`, `value`) VALUES (1, 1, 'dom_param_name', 'dom_param_value');
INSERT INTO `dir_domain_variables` (`id`, `dir_domain_id`, `name`, `value`) VALUES (1, 1, 'dom_variable_name', 'dom_variable_value');
INSERT INTO `dir_groups` (`id`, `name`) VALUES (1, 'prepay'), (2, 'annoying_customers');
INSERT INTO `dir_groups_users` (`dir_group_id`, `dir_user_id`) VALUES (1, 1), (2, 1);
INSERT INTO `dir_users` (`id`, `dir_domain_id`, `username`, `enabled`, `cidr`, `mailbox`, `number-alias`) VALUES (1, 1, 'someuser', 1, NULL, 'test@test.com', '1234');
INSERT INTO `dir_user_params` (`id`, `dir_user_id`, `name`, `value`) VALUES (1, 1, 'password', 'topsecret'), (2, 1, 'vm-password', '0000');
INSERT INTO `dir_user_variables` (`id`, `dir_user_id`, `name`, `value`) VALUES (1, 1, 'accountcode', '1234'), (2, 1, 'user_context', 'default'), (3, 1, 'vm_extension', '1234');

To make FreeSWITCH do directory searches through a Lua script, add the following to lua.conf.xml:

    <param name="xml-handler-script" value="dir_user_xml_multi_table_mysql.lua"/>
    <param name="xml-handler-bindings" value="directory"/>
]]

local req_domain = params:getHeader("domain")
local req_key    = params:getHeader("key")
local req_user   = params:getHeader("user")

if req_key == "id" then -- in this example, I use user.id as auto incrementing primary key
  req_key = "username"
end

assert (req_domain and req_key and req_user,
  "This example script only supports generating directory xml for a single user !\n")

local env = assert(luasql.scdb()) 
local con = assert(env:connect("dsn","user","pass")) 


-- it's probably wise to sanitize input to avoid SQL injections !
local user_query = string.format(
[[SELECT
  dir_domains.id AS domain_id,
  dir_users.id AS user_id,
  dir_users.username,
  dir_users.cidr,
  dir_users.mailbox,
  dir_users.`number-alias`
FROM
  dir_domains,
  dir_users
WHERE
  dir_domains.name = '%s' AND
  dir_users.`%s` = '%s' AND
  dir_users.dir_domain_id = dir_domains.id AND
  dir_users.enabled = '1';
]], req_domain, req_key, req_user)


-- some helper functions

-- name_value_to_xml - returns: <tags><tag name="foo" value="bar"/>...</tags>
local function name_value_to_xml(tag, query)
  local ret = "<" .. tag .. "s>"
  local res = assert(con:exec2table(query))
  for local i = 1, table.getn(res) do
    ret = ret .. "<" .. tag .. " name=\"" .. res[i]["name"] .. "\" value=\"" .. res[i]["value"] .. "\"/>"
  end
  return(ret .. "</" .. tag .. "s>")
end

-- groups_to_xml - returns: <group name="foo"><user id="bar" type="pointer"/></group>...
local function groups_to_xml(user_id, username)
  local ret = ""
  local query = "SELECT name FROM dir_groups, dir_groups_users WHERE" ..
                "dir_groups_users.dir_group_id = dir_groups.id AND dir_user_id = " .. user_id
  local res = assert(con:exec2table(query)) -- yes I'm using con here, it's in scope and it's just an example
  for local i = 1, table.getn(res) do
    ret = ret .. "<group name=\"" .. res[i]["name"] .. "\"><user id=\"" .. username .. "\" type=\"pointer\"/>"
  end
  return ret
end


-- generate the xml

local res = assert(con:exec2table(user_query)) 

if table.getn(res) == 1 then
  local u = user_res[1]
  XML_STRING =
[[<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<document type="freeswitch/xml">
  <section name="directory">
    <domain name="]] .. req_domain .. [[">]] ..
name_value_to_xml("param", "SELECT name, value FROM dir_domain_params WHERE dir_domain_id = " .. u.domain_id) ..
name_value_to_xml("variable", "SELECT name, value FROM dir_domain_variables WHERE dir_domain_id = " .. u.domain_id) .. [[
      <groups>
        <group name="default">
          <users>
            <user id="]] .. u.user_id .. [[" mailbox="]] .. u.mailbox .. [[" cidr="]]
                 .. u.cidr .. [[" number-alias="]] .. u["number-alias"] .. [[">]] ..
name_value_to_xml("param", "SELECT name, value FROM dir_user_params WHERE dir_user_id = " .. u.user_id) ..
name_value_to_xml("variable", "SELECT name, value FROM dir_user_variables WHERE dir_user_id = " .. u.user_id) .. [[
            </user>
          </users>
        </group>]] ..
groups_to_xml(u.user_id, u.username) .. [[
      </groups>
    </domain>
  </section>
</document>]]
end


--[[ When searched, should return:

<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<document type="freeswitch/xml">
  <section name="directory">
    <domain name="test.com">
      <params>
        <param name="dom_param_name" value="dom_param_value"/>
      </params>
      <variables>
        <param name="dom_variable_name" value="dom_variable_value"/>
      </variables>
      <groups>
        <group name="default">
          <users>
            <user id="someuser" mailbox="test@test.com" cidr="" number-alias="1234">
              <params>
                <param name="password" value="topsecret"/>
                <param name="vm-password" value="0000"/>
              </params>
              <variables>
                <variable name="accountcode" value="1234"/>
                <variable name="user_context" value="default"/>
              </variables>
            </user>
          </users>
        </group>
      <group name="prepay"><user id="someuser" type="pointer"/>
      <group name="annoying_customers"><user id="someuser" type="pointer"/>
      </groups>
    </domain>
  </section>
</document>

]]
