-- trim9 from http://lua-users.org/wiki/StringTrim
local _util_lib_find = string.find
local _util_lib_sub = string.sub
function trim(s)
  local _,i1 = _util_lib_find(s,'^%s*')
  local i2 = _util_lib_find(s,'%s*$')
  return _util_lib_sub(s,i1+1,i2-1)
end
