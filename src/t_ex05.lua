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

OddlyEvenStore = require("ex05");

a = OddlyEvenStore.new()
b = OddlyEvenStore.new()

print("the userdata (a)    ", a)     --> OddlyEven(2) @ 0x...
print("the userdata (b)    ", b)     --> OddlyEven(2) @ 0x...
print("the size of userdata", #a)   --> 2

print("a's initial values  ", a[0], a[1]);         --> nil, nil
print("b's initial values  ", b[2], b[3]);         --> nil, nil

-- integer

a[1] = 10;
b[1] = 99;
print("integer             ", a[3], b[3]); --> 10 10 99 99

-- float

a[2] = 12.0
print("float               ", a[4]);  --> 12.0

-- string

function setStr(s, str)
  str = str or "World!";
  s[0] = str;
end

setStr(a, "Hello");
setStr(b);
print("string              ", a[0], b[4]);

-- boolean
a[0] = true;
b[0] = false;

print("boolean             ", a[2], b[4]);

-- table
aa = {a = 42};
bb = {b = 99};
a[0] = aa;
b[0] = bb;
print("table               ", a[0], a[0].a,  b[0], b[0].a)
a[0].a = 99;
print("aa.a changed (to 99)", aa.a);

-- userdata
a[0] = b
print("userdata a[0] = b   ", a[0], "=", b);

-- now __gc will kick in

