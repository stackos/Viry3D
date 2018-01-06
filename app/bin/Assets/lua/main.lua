local lexer = require "lexer"
local lua_lexer = lexer.load("lua")
local tokens = lexer.lex(lua_lexer, 'print(1 + 2)')
for name, pos in pairs(tokens) do
    print(name, pos)
end
print("Hello World!")
print(1 + 2)
function func(a, b)
    print(a + b)
end
func(3, 4)