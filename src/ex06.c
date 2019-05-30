// file ex06.c
// gcc -Isrc -undefined -shared -fPIC -o ex06.so src/ex06.c

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
* Store *anything* in an OddlyEven store (the easy way..)
* - just store *everything* in the LUA_REGISTRY via an integer 'ref'
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

    luaL_getmetatable(L, "ex06.OddlyEven");

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
    void *ud = luaL_checkudata(L, 1, "ex06.OddlyEven");
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
    {"__len", size},
    {"__gc", destroy},
    {NULL, NULL}
};

//luaopen_<libname>
int luaopen_ex06 (lua_State *L)
{
    luaL_newmetatable(L, "ex06.OddlyEven");  // [ M ]
    lua_pushvalue(L, -1);                    // [ M, M ]
    lua_setfield(L, -2, "__index");          // [ M{__index=M} ]
    luaL_setfuncs(L, meths, 0);              // [ M ..} ]
    luaL_newlib(L, funcs);                   // [ M{..} {new=newarray} ]

    return 1;
}
