// file ex08.c
// gcc -Isrc -undefined -shared -fPIC -o ex08.so src/ex08.c

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>

// See lua.h for LUA_TNONE, ... 100 will not clash w/ predefined types
#define OE_TINTEGER 100

/* OddlyEven - store any Lua object in C
* -------------------------------------------------------------------------
* Store *anything* in an OddlyEven store
* - adding an ipairs-iterator
*   for k, v in ipairs(store) do
*     print(k, v)
*   end
*
*/


// debug functions

#include "stackdump.h"  // stackDump(L, "txt")

// the C-datastructure

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

    luaL_getmetatable(L, "ex08.OddlyEven");

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
    void *ud = luaL_checkudata(L, 1, "ex08.OddlyEven");
    luaL_argcheck(L, ud != NULL, 1, "`OddlyEven' storage expected");

    return (Item *)ud;
}

Item *
getItem (lua_State *L)
{   // return ptr to Item in an OddlyEven store, using given index

    // [ ud idx ..]
    Item *p = checkUserDatum(L);
    int idx = luaL_checkinteger(L, 2);

    return p +(idx & 0x1);  // ptr to an Item
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
//    The 'for'-loop calls pairs (the factory function) which returns 3 values
//    that 'for' uses to control the iteration:
//    - the iterator function - called successively w/ (inv.state, control var)
//    - the invariant state - same for all iteration calls
//    - the initial value for the control variable
//  If store has a __pairs metamethod, pairs will simply call that, so in
//  this example, __pairs is the factory function.  If the factory func returns
//  less than 3 values, the missing values are taken to be nil.
//
// 2) Iteration
//    The 'for'-loop then calls the iterator function with (initial value of)
//    the invariant state and the current value of the control variable as its
//    arguments.  The iterator function returns the new value for the control
//    variable and any other values to match the 'exp-list', here 'k,v'.
//
//    Iteration stops once the iterator returns nil for the control variable.
static int
iter_kv(lua_State *L)
{
  // the actual iterator func

  printf("iter_kv:\n");
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
  stackDump(L, "0");               // [ud]
  lua_pushcfunction(L, iter_kv);      // [ud f]
  stackDump(L, "1");
  lua_rotate(L, 1, 1);             // [f ud]
  stackDump(L, "2");
  lua_pushinteger(L, -1);          // [f ud -1]
  stackDump(L, "3");

  return 3; // [iterator_func invariant_state initial_control_value]
}

// ipairs iterator
// A bit of a bad example, since the protocol for ipairs is almost the same
// as for the pairs iterator, except that the index starts at 0 and the fact
// that OddlyEven has no 'string' keys ...  Anyway, so simulate the end of the
// for ipairs loop then 'second' element is mapped to nil.
static int
iter_k(lua_State *L)
{
  // the actual iterator func

  printf("iter_k:\n");
  stackDump(L, "0");                   // [ud idx] == inv.state key

  luaL_checktype(L, 2, LUA_TNUMBER);   // idx needs to be a number
  Item *p = checkUserDatum(L);         // invariant state is the userdatum
  int idx = luaL_checkinteger(L, 2);   // translate number to int

  idx = idx + 1;                       // next available index
  if (idx > 2) return 0;               // will set for's k,v to nil,nil
  lua_pushinteger(L, idx);             // push the control var [.., idx']
  idx &= 0x1;                          // map back to [0,1]
  lua_rawgeti(L, LUA_REGISTRYINDEX, (p+idx)->ref);  // and its value
  if (lua_isnil(L, -1)) return 0;      // stop iteration at nil-value
  stackDump(L, "1");                  // [ud idx idxr'+ value]
  return 2;                           // k,v := [.., idxr'+, value]
}

static int
ipairs(lua_State *L)
{
  // factory function for k,v in pairs(store) do .. end
  // returns: [f ud -1]
  // - iterator C-function,
  // - userdatum as invariant state, and
  // - -1 as the initial control value

  printf("__ipairs factory:\n");
  stackDump(L, "0");                 // [ud]
  lua_pushcfunction(L, iter_k);      // [ud f]
  stackDump(L, "1");
  lua_rotate(L, 1, 1);               // [f ud]
  stackDump(L, "2");
  lua_pushinteger(L, 0);             // [f ud 0]
  stackDump(L, "3");

  return 3; // [iterator_func invariant_state initial_control_value]
}
//  REGISTER LIBRARY

// the module's functions
static const struct luaL_Reg funcs [] = {
    {"new", new},
    {NULL, NULL}
};

// the object's (meta) methods
static const struct luaL_Reg meths [] = {
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
int luaopen_ex08 (lua_State *L)
{
    luaL_newmetatable(L, "ex08.OddlyEven");  // [ M ]
    lua_pushvalue(L, -1);                    // [ M, M ]
    lua_setfield(L, -2, "__index");          // [ M{__index=M} ]
    luaL_setfuncs(L, meths, 0);              // [ M ..} ]
    luaL_newlib(L, funcs);                   // [ M{..} {new=newarray} ]

    return 1;
}
