#!/usr/local/bin/lua
--
--------------------------------------------------------------------------------
--         File:  t_ex03.lua
--
--        Usage:  src/t_ex03.lua
--
--  Description:  
--
--      Options:  ---
-- Requirements:  ---
--         Bugs:  ---
--        Notes:  ---
--       Author:  YOUR NAME (), <>
-- Organization:  
--      Version:  1.0
--      Created:  26-05-19
--     Revision:  ---
--------------------------------------------------------------------------------
--

package.cpath = "bld/?.so"
array = require("ex03");

a = array.new(10)

print(a)               --> userdata: 0x8064d48
print(array.size(a))   --> 10

for i=1,10 do
  array.set(a, i, 1/i)
end

print(array.get(a, 10))  --> 0.1

print("Error follows due to: array.get(io.stdin, 1)");
array.get(io.stdin, 1);


--[[ Note:

Can't do the following with the library 'ex02.so'

a:set(2, 10);
print(a:get(2))

lua: src/t_ex02.lua:35: attempt to index a userdata value (global 'a')
stack traceback:
  src/t_ex02.lua:35: in main chunk
  [C]: in ?

A call like: print(a[2]) produces the same error.

--]]

