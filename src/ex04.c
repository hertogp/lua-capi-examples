// file ex04.c
// gcc -Isrc -undefined -shared -fPIC -o ex04.so src/ex04.c

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <stdio.h>
#include <assert.h>

/* https://www.lua.org/pil/28.3.html
* -------------------------------------------------------------------------
* Turn array to an object in Lua
* - only export the new function, the other functions go into the metatable
*   with new metamethod names like __len, __index, __newindex, etc..
*
*/

// debug functions

#include "stackdump.h"

// the C-datastructure

typedef struct NumArray {
  int size;
  double values[1];  /* variable part */
} NumArray;

static NumArray *
checkarray (lua_State *L)
{
    // check 1st argument (valid userdatum) & return as ptr to NumArray
    void *ud = luaL_checkudata(L, 1, "ex04.array");
    luaL_argcheck(L, ud != NULL, 1, "`array' expected");

    return (NumArray *)ud;
}

static double *
getelem (lua_State *L)
{
    // check 2nd argument (valid integer) & return ptr to indexed array elm
    NumArray *a = checkarray(L);
    int index = luaL_checkinteger(L, 2);

    luaL_argcheck(L, 1 <= index && index <= a->size, 2,
            "index out of range");

    /* return element address */
    return &a->values[index - 1];
}

// the array library

static int
newarray (lua_State *L)
{
    // Due to the fact that in C89 you cannot declare an array of 0, we need
    // to use an array of 1 elm in NumArray, so:
    // - sizeof(NumArray) == sizeof(int) + sizeof(double)
    // - hence, the (n-1)*sizeof(double)
    // - so nbytes == sizeof(int) + sizeof(double) + (n-1)*sizeof(double)
    //             == sizeof(int) + n*sizeof(double)


    int n = luaL_checkinteger(L, 1);
    printf("newarray\n");
    stackDump(L, "1");                // [n]

    size_t nbytes = sizeof(NumArray) + (n - 1)*sizeof(double);
    NumArray *a = (NumArray *)lua_newuserdata(L, nbytes);
    stackDump(L, "2");               // [n, ud]
    luaL_getmetatable(L, "ex04.array");
    stackDump(L, "3");              // [n, ud{}, M{}]
    printf("Top elm %p\n", lua_touserdata(L,-2));
    printf("Top elm %p\n", lua_touserdata(L,-1));
    lua_setmetatable(L, -2);
    stackDump(L, "4");              // [n, ud{}]
    a->size = n;

    return 1;  /* new userdatum is already on the stack */
}

static int
setarray (lua_State *L)
{   // [userdata index value]
    double newval = luaL_checknumber(L, 3);
    *getelem(L) = newval;

    return 0;
}

static int
getarray (lua_State *L)
{
    lua_pushnumber(L, *getelem(L));

    return 1;
}

static int
getsize (lua_State *L)
{
  NumArray *a = checkarray(L);
  lua_pushnumber(L, a->size);

  return 1;
}

// __tostring method
int
array2string (lua_State *L)
{
    NumArray *a = checkarray(L);
    lua_pushfstring(L, "array(%d)", a->size);
    return 1;
}

//  REGISTER LIBRARY

static const struct luaL_Reg funcs [] = {
    {"new", newarray},
    {NULL, NULL}
};

static const struct luaL_Reg meths [] = {
    {"__tostring", array2string},
    {"__newindex", setarray},
    {"__index", getarray},
    {"__len", getsize},
    {NULL, NULL}
};

//luaopen_<name_as_required>
int luaopen_ex04 (lua_State *L)
{
    /* Initialize the library:
     * - newmetatable creates, registers (name=address) & pushes tbl onto stack
     * - next the metatable (M) (meta)method(s) get set
     * - lastly, an anonymous table is created w/ the library functions
     * in Lua:
     *   array = require("ex04") -> store anonymous table (library) in a var
     */

    printf("luaopen_ex04\n");             // []
    stackDump(L, "start");
    luaL_newmetatable(L, "ex04.array");  // [ M{} ]
    stackDump(L, "newmetatable");
    lua_pushvalue(L, -1);                // [ M{}, M{} ]
    stackDump(L, "pushvalue");
    lua_setfield(L, -2, "__index");      // [ M{__index=M} ]
    stackDump(L, "setfield __index");
    luaL_setfuncs(L, meths, 0);          // [ M{__index=M, set=setarray, ..} ]
    stackDump(L, "setfuncs");

    luaL_newlib(L, funcs);               // [ M{..} {new=newarray} ]
    stackDump(L, "newlib");
    printf("\n");

    return 1;
}
