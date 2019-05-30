#!/usr/local/bin/lua
--
--------------------------------------------------------------------------------
--         File:  t_ex04.lua
--
--        Usage:  src/t_ex04.lua
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
array = require("ex04");

a = array.new(10)

print(a)               --> userdata: 0x8064d48

print(#a)              --> 10

for i=1,10 do
  a[i] = 1/i;
end

print(a[10])           --> 0.1

