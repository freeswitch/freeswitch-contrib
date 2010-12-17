function string:split(delimiter)
  local result = {}
  local from = 1
  local delim_from, delim_to = string.find(self, delimiter, from)
  while delim_from do
    table.insert(result, string.sub(self, from , delim_from-1))
    from = delim_to + 1
    delim_from, delim_to = string.find(self, delimiter, from)
  end
  table.insert(result, string.sub(self, from))
  return result
end

function string:wrap(boundary, indent)
  local output = {}
  local index, words, leading_space, full_indent
  local indent = indent or ""
  local buffer = indent
  local lines = self:split("\n") 
  
  for _, line in pairs(lines) do
    if line:match("^%s*$") then
      table.insert(output, "")
    else
      words = line:split(" ")
      leading_space = line:match("^(%s+)%S")
      if (#words > 0) then
        if leading_space then
          full_indent = leading_space .. indent
        else
          full_indent = indent
        end
        index = 1
        while words[index] do
          local word = " " .. words[index]
          if (buffer:len() >= boundary) then
            table.insert(output, buffer:sub(1, boundary))
            buffer = full_indent .. buffer:sub(boundary + 1)
          else
            if (word:len() > boundary) then
              buffer = buffer .. word
            elseif (buffer:len() + word:len() >= boundary) then
              table.insert(output, buffer)
              buffer = full_indent
            else
              if (buffer == full_indent) then
                buffer = full_indent .. word:sub(2)
              else
                buffer = buffer .. word
              end
              index = index + 1
            end
          end
        end
        if (buffer:match("%S")) then
            table.insert(output, buffer)
            buffer = indent
        end
      end
    end
  end
  return table.concat(output, "\n")
end

