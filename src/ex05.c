// file ex05.c
// gcc -Isrc -undefined -shared -fPIC -o ex05.so src/ex05.c

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
* Store *anything* in an OddlyEven store (the hard way..)
* - scalars are directly (calloc, cast & assign)
* - anything else a ref(erence) int which is a key i/t LUA_REGISTRY which
*   stores the non-scalar value for us.
*
* A value is stored as an Item:  struct Item { int type, void *val};
*
* itm->type = lua_type(L, x); in case of LUA_TNUMBER,
* if lua_isinteger(L, x) the itm->type = OE_TINTEGER to differentiate between
* integers and floats, which both have type LUA_TNUMBER
*
* Storing any value entails free'ing the existing entry and allocate new memory
* based on itm->type, which is summarized below:
* - LUA_TNONE (-1)         - no valid value @ some stackindex
* - LUA_TNIL  (0)          - itm->val = NULL
* - LUA_TBOOLEAN (1)       - *(int *)itm->val = lua_toboolean(org)
* - LUA_TLIGHTUSERDATA (2) - *(int *)itm->val = luaL_ref(L, LUA_REGISTRYINDEX)
* - LUA_TNUMBER (3)        - *(lua_Integer *)itm->val = luaL_tointeger(arg)
*   OE_TINTEGER (100)      - *(lua_Number *)itm->val = luaL_tonumber(arg)
* - LUA_TSTRING (4)        - strncpy(itm->val, s, len+1)
* - LUA_TTABLE (5)         - *(int *)itm->val = luaL_ref(L, LUA_REGISTRYINDEX)
* - LUA_TFUNCTION (6)      - *(int *)itm->val = luaL_ref(L, LUA_REGISTRYINDEX)
* - LUA_TUSERDATA (7)      - *(int *)itm->val = luaL_ref(L, LUA_REGISTRYINDEX)
* - LUA_TTHREAD (8)        - *(int *)itm->val = luaL_ref(L, LUA_REGISTRYINDEX)
* and
* - LUA_NUMTAGS (9) ?
*
*/


// debug functions

#include "stackdump.h"  // stackDump(L, "txt")

// the C-datastructure

typedef struct Item {
    int type;
    void *val;
} Item;

// the library

static int
new (lua_State *L)
{
    // create userdatum, its metatable & set it.
    Item *p = (Item *)lua_newuserdata(L, 2 * sizeof(Item));
    luaL_getmetatable(L, "ex05.OddlyEven");
    lua_setmetatable(L, -2);

    // init the structure
    p->type = (p+1)->type = LUA_TNIL;
    p->val = (p+1)->val = NULL;

    return 1;  /* new userdatum is already on the stack */
}

// auxiliary functions

Item *
checkUserDatum (lua_State *L)
{
    // check 1st argument is a valid OddlyEven store & return a ptr to it.
    void *ud = luaL_checkudata(L, 1, "ex05.OddlyEven");
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

    // free current value, if any
    if (itm->type == LUA_TTABLE       ||
        itm->type == LUA_TTHREAD      ||
        itm->type == LUA_TFUNCTION    ||
        itm->type == LUA_TUSERDATA    ||
        itm->type == LUA_TNUMBER      ||
        itm->type == LUA_TLIGHTUSERDATA)
        luaL_unref(L, LUA_REGISTRYINDEX, *(int *)itm->val);

    if (itm->val) {
        free(itm->val);
        itm->val = NULL;
        itm->type = LUA_TNONE;
    }

    // set type-specific value's
    // int lua_type (lua_State *L, int index);        [-0, +0, –]
    // Returns:
    //  - the type of the value in the given valid index, or
    //  - LUA_TNONE for a non-valid (but acceptable) index.
    //
    //  The types returned by lua_type are coded by the lua.h constants:
    //  a) LUA_TNIL (0)
    //  b) LUA_TNUMBER, LUA_TBOOLEAN, LUA_TSTRING,
    //  c) LUA_TTABLE, LUA_TFUNCTION, LUA_T(LIGHT)USERDATA, LUA_TTHREAD,
    //
    //  a) means a nil value and assigning nil to a key, means deletion.
    //  b) can be copied as values to memory
    //  c) need to be stored in the LUA_REGISTRYINDEX

    itm->type = lua_type(L, 3);
    switch(itm->type)
    {
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TTHREAD:
        case LUA_TUSERDATA:
        case LUA_TLIGHTUSERDATA:
            {
                itm->val = calloc(1, sizeof(int));
                *(int *)itm->val = luaL_ref(L, LUA_REGISTRYINDEX);
                break;
            }
        case LUA_TBOOLEAN:
            {
                itm->val = calloc(1, sizeof(int));
                *(int *)itm->val = lua_toboolean(L, 3);
                break;
            }
        case LUA_TNUMBER:
            {
                if (lua_isinteger(L, 3)) {
                    itm->val = calloc(1, sizeof(lua_Integer));
                    *(lua_Integer *)itm->val = lua_tointeger(L, 3);
                    itm->type = OE_TINTEGER;
                } else {
                    itm->val = calloc(1, sizeof(lua_Number));
                    *(lua_Number *)itm->val = luaL_checknumber(L, 3);
                }
                break;
            }
        case LUA_TSTRING:
            {
                // use len & strncpy rather than strdup, since s may contain
                // '\0's and we need a full copy..
                size_t len = 0;
                const char *s = lua_tolstring(L, 3, &len); // volatile
                if (s != NULL) {
                    itm->val = calloc(len + 1, sizeof(char));
                    strncpy(itm->val, s, len + 1);
                }
                break;
            }
        case LUA_TNIL:
            {
                break;  // itm->value already NULL;
            }
        case LUA_TNONE:
            {
                break; // acceptable but non-valid index into the stack
            }
        default:
            {
                printf("not supported type %d\n", itm->type);
                break;
            }
    }

    return 0; // no Lua facing return results
}

static int
get (lua_State *L)
{
    // [ud idx]
    Item *itm = getItem(L);
    switch(itm->type)
    {
        case LUA_TTABLE:
        case LUA_TFUNCTION:
        case LUA_TTHREAD:
        case LUA_TUSERDATA:
        case LUA_TLIGHTUSERDATA:
            {
                lua_rawgeti(L, LUA_REGISTRYINDEX, *(int *)itm->val);
                break;
            }
        case LUA_TBOOLEAN:
            {
                lua_pushboolean(L, *(int *)itm->val);
                break;
            }
        case LUA_TNUMBER:
            {
                lua_pushnumber(L, *(lua_Number *)itm->val);
                break;
            }
        case OE_TINTEGER:
            {
                lua_pushinteger(L, *(lua_Integer *)itm->val);
                break;
            }
        case LUA_TSTRING:
            {
                lua_pushstring(L, (const char *)itm->val);
                break;
            }
        default:
            {
                lua_pushnil(L);
                break;
            }
    }
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
    stackDump(L, "1");
    lua_pushinteger(L, 0);
    lua_pushnil(L);
    stackDump(L, "2");
    set(L);
    lua_pop(L, 2);

    // clear Odd entry
    printf("  Odd entry\n");
    lua_pushinteger(L, 1);
    lua_pushnil(L);
    stackDump(L, "3");
    set(L);
    lua_pop(L, 2);
    stackDump(L, "4");
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
int luaopen_ex05 (lua_State *L)
{
    luaL_newmetatable(L, "ex05.OddlyEven");  // [ M ]
    lua_pushvalue(L, -1);                    // [ M, M ]
    lua_setfield(L, -2, "__index");          // [ M{__index=M} ]
    luaL_setfuncs(L, meths, 0);              // [ M ..} ]
    luaL_newlib(L, funcs);                   // [ M{..} {new=newarray} ]

    return 1;
}
