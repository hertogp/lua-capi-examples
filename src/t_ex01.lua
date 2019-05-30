#!/usr/local/bin/lua
--
------------------------------------------------------------------------------
--         File:  ex01.lua
--
--        Usage:  ./ex01.lua
--
--  Description:  Lua C-api example 01
--
--      Options:  ---n/a
-- Requirements:  ---n/a
--         Bugs:  ---
--        Notes:  ---Run this from root dir (src/t_ex01.lua)
--       Author:  YOUR NAME (), <>
-- Organization:  
--      Version:  1.0
--      Created:  26-05-19
--     Revision:  ---
------------------------------------------------------------------------------
--

package.cpath = "bld/?.so"
c = require("ex01");

print();

print("c.c_sin(2) --> ", c.c_sin(2));

local a = "1";
local b = 3;

print("\na, b = c.c_swap(a, b)")
print(string.format("- before a %d, b %d", a, b));
a , b = c.c_swap(a,b);
print(string.format("- after  a %d, b %d", a, b));

print();

-- thanks to arg checking in C:
--
-- calling c.c_sin("oops), yields:
-- lua: src/t_ex01.lua:27: bad argument #1 to 'c_sin' (number expected, got string)
--
-- and c.c_swap("oops", 2) yields:
-- lua: src/t_ex01.lua:34: bad argument #1 to 'c_swap' (number expected, got string)

