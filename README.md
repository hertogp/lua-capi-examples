# Lua C API Examples

A small collection of examples on how to extend Lua with C libraries.

# Makefile

The setup:

```bash
.
├── LICENSE
├── Makefile
├── README.md
├── bld
├── inc
│   ├── lauxlib.h
│   ├── lua.h
│   ├── luaconf.h
│   └── lualib.h
└── src
    ├── ex01.c
    ├── ex02.c
    .
    ├── ex10.c
    ├── stackdump.h
    ├── t_ex01.lua
    ├── t_ex02.lua
    .
    └── t_ex10.lua

3 directories, 29 files
```

There is one `src` directory for all example code, both C and Lua.  All
includes are located in `inc` and all make artifacts go into `bld`.

All Lua `t_ex<nr>.lua` test scripts add `bld/?.so` to their `package.cpath`.

Tests are run from the project's root directory:via `make ex<nr>`.  A `make
tests` runs them all.  All test runs use valgrind.

```bash
# pick up examples in src subdir
EXAMPLES=$(sort $(wildcard src/*.c))

# filename sans .c become targets
TARGETS=$(EXAMPLES:src/%.c=%)

RM=/bin/rm

# build/run an example
$(TARGETS): %: src/%.c
	@echo "Example $@"
	$(CC) -Iinc -undefined -shared -fPIC -o bld/$@.so $<
	@echo "Calling src/t_$@.lua"
	valgrind --leak-check=yes lua src/t_$@.lua

tests:
	@$(foreach target, $(TARGETS), make $(target);)

# alternative build sequence (ex01.c example):
#    $(CC) -Iinc -fPIC -shared -c src/ex01.c -o bld/ex01.o
#    $(CC) -fPIC -shared -undefined bld/ex01.o -o ex01.so
#    ./t_ex01.lua

clean:
	$(RM) -f bld/*.so bld/*.o
```

# The examples

- ex01, calling C-functions from Lua
- ex02, Lua's array example without metatable to check userdata type
- ex03, Lua's array example with metatable to check userdata type
- ex04, Lua's array example with object-like syntax on the Lua side
- ex05, OddlyEven, store anything, the hard way
- ex06, OddlyEven, store anything, the easy way
- ex07, OddlyEven, add `pairs()` iterator
- ex08, OddlyEven, add `ipairs()` iterator
- ex09, OddlyEven, add `values()` iterator
- ex10, OddlyEven, add `keys()` iterator

All examples use the stackDump(lua_State *L, const char *) from
`inc/stackdump.h` to dump the stack at various points in particular functions.
Data is marshalled by the 'virtual stack` and the main hurdle is figuring out
how that works, specifically how to best manipulate the stack.

Anyway, the examples build on top of each other, so here's ex10:

```c
// file ex10.c
// gcc -Isrc -undefined -shared -fPIC -o ex10.so src/ex10.c

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

/* OddlyEven - store any Lua object in C
* ----------------------------------------------------
* Store *anything* in an OddlyEven store
* - adding a keys()-iterator
*   for k in store:keys() do
*     print(k)
*   end
*
*/


// debug functions

#include "stackdump.h"  // stackDump(L, "txt")

// the C-datastructure
// The userdatum will be a pointer to two Items in memory,
// indexed by a natural integer which is mapped to the range
// of [0,1] using a mask of 0x1;  Normally other data may be
// included in an entry, so we'll keep the struct envelope.

typedef struct Item {
    int ref; // the key into the LUA_REGISTRY for this item
} Item;

// the library

static int
new (lua_State *L)
{
    // create userdatum, its metatable & set it.

    printf("OddlyEven new:\n");
    stackDump(L, "1");    // []

    Item *p = (Item *)lua_newuserdata(L, 2 * sizeof(Item));

    stackDump(L, "2");    // [ud]

    luaL_getmetatable(L, "ex10.OddlyEven");

    stackDump(L, "3");    // [ud M]

    lua_setmetatable(L, -2);

    stackDump(L, "4");    // [ud]

    // init the structure
    p->ref = (p+1)->ref = LUA_NOREF;  // different from any valid reference

    return 1;  // new userdatum is already on the stack
}

// auxiliary functions

Item *
checkUserDatum (lua_State *L)
{
    // check 1st argument is a valid OddlyEven store & return a ptr to it.
    void *ud = luaL_checkudata(L, 1, "ex10.OddlyEven");
    luaL_argcheck(L, ud != NULL, 1, "`OddlyEven' storage expected");

    return (Item *)ud;
}

static int values(lua_State *);

Item *
getItem (lua_State *L)
{   // return ptr to Item in an OddlyEven store, using given index

    // [ ud idx ..]
    printf("getItem:\n");
    stackDump(L, "initial stack - 0");
    Item *p = checkUserDatum(L);

    if (lua_isnumber(L, 2)) {
      // numbered index -> Stack [ud idx val]  (val only when setting)
      int idx = lua_tointeger(L, 2);
      stackDump(L, "return Item* - 1a");
      return p +(idx & 0x1);            // map idx to [0,1] & return ptr Item
    } else if (lua_isstring(L, 2)) {
      // method name  -> Stack:    [ud name]
      lua_getmetatable(L, 1);   // [ud name M]
      lua_rotate(L, 2, 1);      // [ud M name]
      lua_gettable(L, -2);      // [ud M func]
      stackDump(L, "return NULL - 1b");
      return NULL;
    } else {
      lua_pushliteral(L, "error accessing OddlyEven!");
      lua_error(L);
    }
    return p; // NOT REACHED
}
static int
set (lua_State *L)
{
    // [ud num val]
    Item *itm = getItem(L);

    // free reference (if any)
    luaL_unref(L, LUA_REGISTRYINDEX, itm->ref);
    itm->ref = luaL_ref(L, LUA_REGISTRYINDEX);

    return 0; // no Lua facing return results
}

static int
get (lua_State *L)
{
    // [ud idx]
    Item *itm = getItem(L);
    if (itm == NULL) { // no valid index (e.g. func name)
      if (lua_iscfunction(L, -1))
          return 1;  // return method
      else
          return 0;  // non-existing entry
    }

    if (itm->ref == LUA_NOREF)
        return 0;

    lua_rawgeti(L, LUA_REGISTRYINDEX, itm->ref);
    return 1;
}

static int
size (lua_State *L)
{
  lua_pushinteger(L, 2);
  return 1;
}

// __tostring method
int
toString (lua_State *L)
{
    Item *p = checkUserDatum(L);
    lua_pushfstring(L, "OddlyEven(2) @ %p", p);
    return 1;
}

// __gc garbage collection, our userdata has become unreachable
static int
destroy(lua_State *L)
{
    Item *p = checkUserDatum(L);
    printf("__gc -> destroy @ %p\n", p);
    printf("  Even entry\n");
    // clear Even entry
    luaL_unref(L, LUA_REGISTRYINDEX, p->ref);

    // clear Odd entry
    luaL_unref(L, LUA_REGISTRYINDEX, (p+1)->ref);

    // no need to free *p itself, that's userdata freed by Lua's gc.
    return 0;
}

// pairs-iterator

// Iterator protocol: for k,v in pairs(store) do ... end
// k = the control var
// 1) Initialization
//    The 'for'-loop calls pairs (the factory function) which
//    returns 3 values that 'for' uses to control the iteration:
//    - the iterator function - called by for successively
//    - the invariant state - 1st arg for iterator
//    - the control variable's initial value - 2nd arg for iterator
// 
//  If store has a __pairs metamethod, pairs will simply call that,
//  so in this example, __pairs is the factory function.  If the
//  factory func returns less than 3 values, the missing values are
//  taken to be nil.
//
// 2) Iteration
//    The 'for'-loop calls the iterator function with (initial
//    value of) the invariant state and the current value of the
//    control variable as its arguments.  The iterator function
//    returns the new value for the control variable and any other
//    values to match the 'exp-list', here 'k,v'.
//
//    Iteration stops once the iterator returns nil for the control
//    variable.

static int
iter_pairs(lua_State *L)
{
  // the actual iterator func

  printf("iter_pairs:\n");
  stackDump(L, "0");                   // [ud idx] == inv.state key

  luaL_checktype(L, 2, LUA_TNUMBER);   // idx needs to be a number
  Item *p = checkUserDatum(L);         // invariant state is the userdatum
  int idx = luaL_checkinteger(L, 2);   // translate number to int

  idx = (idx < 0) ? 0 : idx + 1;       // next available index
  if (idx > 1) return 0;               // will set for's k,v to nil,nil

  lua_pushinteger(L, idx);             // push the control var
  lua_rawgeti(L, LUA_REGISTRYINDEX, (p+idx)->ref);  // and its value
  stackDump(L, "1");                  // [ud idx idx+1 value]
  return 2;                           // k,v := idx+1, value
}

static int
pairs(lua_State *L)
{
  // factory function for k,v in pairs(store) do .. end
  // returns: [f ud -1]
  // - iterator C-function,
  // - userdatum as invariant state, and
  // - -1 as the initial control value

  printf("__pairs factory:\n");
  stackDump(L, "0");                // [ud]
  lua_pushcfunction(L, iter_pairs); // [ud f]
  stackDump(L, "1");
  lua_rotate(L, 1, 1);              // [f ud]
  stackDump(L, "2");
  lua_pushinteger(L, -1);           // [f ud -1]
  stackDump(L, "3");

  return 3; // [iterator_func invariant_state initial_control_value]
}

// ipairs iterator

// OddlyEven is A bit of a bad example for this since we have a weird
// mapping of indices to 0 or 1 and no string keys.  Anyway, the
// protocol to follow is similar: the factory func __ipairs returns
// [iter_f ud idx], the last is usually nil to start with, but not
// so in this case.

static int
iter_ipairs(lua_State *L)
{
  // the actual iterator func

  printf("iter_ipairs:\n");
  stackDump(L, "0");                   // [ud idx] == [state key]

  luaL_checktype(L, 2, LUA_TNUMBER);   // idx needs to be a number
  Item *p = checkUserDatum(L);         // invariant state is the userdatum
  int idx = luaL_checkinteger(L, 2);   // translate number to int

  idx = idx + 1;                       // next available index
  if (idx > 2) return 0;               // we're done
  lua_pushinteger(L, idx);             // push the control var [.., idx']
  idx &= 0x1;                          // map idx back to [0,1]

  // get value associated with the ref in slot idx in OddlyEven
  // and push it onto the stack
  lua_rawgeti(L, LUA_REGISTRYINDEX, (p+idx)->ref);

  if (lua_isnil(L, -1)) return 0;      // stop iteration at nil-value
  stackDump(L, "1");                  // [ud idx idx' value]
  return 2;                           // k,v := [.., idx', value]
}

static int
ipairs(lua_State *L)
{
  // factory function for k,v in ipairs(store) do .. end
  // returns: [f ud -1]
  // - iterator C-function,
  // - userdatum as invariant state, and
  // - 0 as the initial control value

  printf("__ipairs factory:\n");
  stackDump(L, "0");                 // [ud]
  lua_pushcfunction(L, iter_ipairs); // [ud f]
  stackDump(L, "1");
  lua_rotate(L, 1, 1);               // [f ud]
  stackDump(L, "2");
  lua_pushinteger(L, 0);             // [f ud 0]
  stackDump(L, "3");

  return 3; // [iterator_func invariant_state initial_control_value]
}


// values()-iterator

static int
iter_values(lua_State *L)
{
  printf("iter_values:\n");
  stackDump(L, "initial stack - 0");   // [ud oldval] (1st time, oldval=nil)
  Item *p = checkUserDatum(L);
  int idx = lua_tointeger(L, lua_upvalueindex(1));  // get upvalue(1)
  if (idx < 0 || idx > 1) return 0;  // we're done

  lua_pushinteger(L, idx + 1);              // [ud oldval idx']
  stackDump(L, "pushed nex idx - 1");
  lua_replace(L, lua_upvalueindex(1));
  stackDump(L, "set upvalue(1) - 2");  // [ud val]-> upvalue(1) = idx'
  idx &= 0x01;                       // map to [0,1]
  lua_rawgeti(L, LUA_REGISTRYINDEX, (p+idx)->ref);  // value by reg-ref
  stackDump(L, "pushed val - 3");                 // [ud old-val val]
  return 1;
}

static int
values(lua_State *L)
{
  // factory function for iterating only the values
  // values() should return a closure func with 1 upvalue: the current index.
  printf("values():\n");
  stackDump(L, "0");                    // [ud]
  lua_pushinteger(L, 0);                // [ud idx]
  lua_pushcclosure(L, iter_values, 1);  // [ud f]
  lua_rotate(L, 1, 1);                  // [f ud] -- iter_func invariant.state
  stackDump(L, "1");

  return 2;
}

// keys()-iterator

static int
iter_keys(lua_State *L)
{
  printf("iter_keys:\n");
  stackDump(L, "initial stack - 0");     // [ud oldval] (1st time, oldval=nil)
  int idx = lua_tointeger(L, lua_upvalueindex(1));  // get upvalue(1)
  if (idx < -1 || idx > 0) return 0;     // we're done
  idx += 1;                              // next valid key in [0, 1]
  lua_pushinteger(L, idx);               // [ud idx idx']
  stackDump(L, "pushed next idx - 1");
  lua_replace(L, -2);                    // [ud idx']
  stackDump(L, "pushed idx again - 2");
  lua_pushinteger(L, idx);               // [ud idx' idx']
  stackDump(L, "pushed 2nd time - 3");
  lua_replace(L, lua_upvalueindex(1));
  stackDump(L, "set upvalue(1) - 4");    // [ud idx']
  return 1;
}

static int
keys(lua_State *L)
{
  // factory function for iterating only the keys
  // keys() returns a closure func with 1 upvalue: the current index.
  // and the invariant state for the iteration
  printf("keys():\n");
  stackDump(L, "0");                   // [ud]
  lua_pushinteger(L, -1);              // [ud nil]  - start w/ nil
  lua_pushcclosure(L, iter_keys, 1);   // [ud f]
  lua_rotate(L, 1, 1);                 // [f ud] -- iter_func invariant.state
  stackDump(L, "1");

  return 2;
}

//  REGISTER LIBRARY

// the module's functions
static const struct luaL_Reg funcs [] = {
    {"new", new},
    {NULL, NULL}
};

// the object's (meta) methods
static const struct luaL_Reg meths [] = {
    {"keys", keys},
    {"values", values},
    {"__tostring", toString},
    {"__newindex", set},
    {"__index", get},
    {"__pairs", pairs},
    {"__ipairs", ipairs},
    {"__len", size},
    {"__gc", destroy},
    {NULL, NULL}
};

//luaopen_<libname>
int luaopen_ex10 (lua_State *L)
{
    luaL_newmetatable(L, "ex10.OddlyEven");  // [ M ]
    lua_pushvalue(L, -1);                    // [ M, M ]
    lua_setfield(L, -2, "__index");          // [ M{__index=M} ]
    luaL_setfuncs(L, meths, 0);              // [ M ..} ]
    luaL_newlib(L, funcs);                   // [ M{..} {new=newarray} ]

    return 1;
}
```

