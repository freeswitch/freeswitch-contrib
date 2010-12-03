--[[
  Helper functions for database operations.
]]

--[[
  Escapes special characters for the MySQL database.
]]
function mysql_escape(string_to_escape)
  -- Single quote, double quote, backspace.
  escaped_string, replacements = string.gsub(string_to_escape, "(['\"\\])", function(x) return "\\" .. x end)
  -- Null byte.
  escaped_string, replacements = string.gsub(escaped_string, "%z", "\\0")

  return escaped_string
end

--[[
  Escapes special characters for the Postgres database.
]]
function pgsql_escape(string_to_escape)
  -- Single quote..
  escaped_string, replacements = string.gsub(string_to_escape, "'", "''")
  -- Null byte.
  escaped_string, replacements = string.gsub(escaped_string, "%z", "")

  return escaped_string
end

--[[
  Escapes special characters for the SQLite database.
]]
function sqlite_escape(string)
  -- Single quote.
  string = string.gsub(string, "'", "''")
  -- Null byte.
  string = string.gsub(string, "%z", "")
  return string
end

