--[[
 *  File:    gs_phonebook.lua
 *  Purpose: Generate a downloadable phonebook for Grandstream phones.
 *  Machine:                      OS:
 *  Author:  John Wehle           Date: June 25, 2009
 ]]--

--[[
 *  Copyright (c)  2009  Feith Systems and Software, Inc.
 *                      All Rights Reserved
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 * 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ]]--


require('LuaXml')


-- RE to control which extensions are published
local extRE = "^10[0-9][0-9]$"

local gs_phonebook = "/tftpboot/gs_phonebook.xml"
local maxEntries = 100


function generate_phonebook ()

  local api = freeswitch.API ()

  local domain = api:execute ("global_getvar", "domain")

  local dir = api:execute ("xml_locate", "directory domain name " .. domain)
  local tdir = xml.eval (dir)

  local tuser = xml.find (xml.find (xml.find (tdir,
                          "groups"), "group"), "users")

  local i
  local k1
  local v1
  local name
  local fname
  local lname
  local number

  local fd = io.open (gs_phonebook .. ".new", "w+")

  io.output (fd)

  io.write ("<?xml version=\"1.0\"?>\n<AddressBook>\n")

  i = 0

  for k1, v1 in pairs (tuser) do
    if type (v1) == "table" and v1[xml.TAG] == "user" then
      name = xml.find (v1, "variable", "name", "effective_caller_id_name")
      number = xml.find (v1, "variable", "name", "effective_caller_id_number")
      if name ~= nil and number ~= nil
         and string.find (number["value"], extRE) ~= nil then

        string.gsub (name["value"], "^[A-Za-z0-9]+",
          function (w) fname = w end, 1)

        string.gsub (name["value"], "[A-Za-z0-9]+$",
          function (w) lname = w end, 1)

        io.write ("  <Contact>\n")
        io.write (string.format ("    <LastName>%s</LastName>\n", lname))
        io.write (string.format ("    <FirstName>%s</FirstName>\n", fname))
        io.write ("    <Phone>\n")
        io.write (string.format ("      <phonenumber>%s</phonenumber>\n",
                                 number["value"]))
        io.write ("      <accountindex>0</accountindex>\n")
        io.write ("    </Phone>\n")
        io.write ("  </Contact>\n")

        i = i + 1

        if i >= maxEntries then
          break
        end
      end
    end
  end

  io.write ("</AddressBook>\n")

  io.close (fd)

  os.execute ("/bin/chmod 444 " .. gs_phonebook .. ".new")

  os.rename (gs_phonebook .. ".new", gs_phonebook)
end


-- Refresh the phonebook on startup
generate_phonebook ()

-- and whenever the configuration changes
local con = freeswitch.EventConsumer ("reloadxml")

while con:pop(1) do
  generate_phonebook ()
end

return
