#!/usr/local/bin/lua
--
--------------------------------------------------------------------------------
--         File:  t_ex09.lua
--
--        Usage:  src/t_ex09.lua
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

OddlyEvenStore = require("ex09");

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

-- iterate OddlyEven  for k,v in pairs() do .. end
a[0] = 11;
a[1] = 99;

print();
print("for-pairs  ".. string.rep("-", 45));
print("for k,v in pairs(a) do print('>> key', v, 'value', v) end\n");
for k,v in pairs(a) do
  print(">> key", k, "value", v)
end
print(string.rep("-", 55), "\n");

-- iterate OddlyEven  for k,v in ipairs() do .. end

print();
print("for-ipairs  ".. string.rep("-", 45));
print("for k,v in ipairs(a) do print('>> key', v, 'value', v) end\n");
for k,v in ipairs(a) do
  print(">> key", k, "value", v)
end
print("\n setting a[1] = nil and reiterating");
a[0] = nil;
for k,v in ipairs(a) do
  print(">> key", k, "value", v)
end

print(string.rep("-", 55), "\n");

-- iterate OddlyEven  for v in a:values() do .. end

print();
print("for-values  ".. string.rep("-", 45));
print("for v in a:values() do print('>> key', v, 'value', v) end\n");
print("setting a[2]=42, a[5]=44 and iterate object a");
a[2] = "fourty-2";
a[5] = "fourty-4";
for v in a:values() do
  print(">> value", v)
end

-- now __gc will kick in

