// file ex01.c
// gcc -Isrc -undefined -shared -fPIC -o ex01.so src/ex01.c

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <stdio.h>

/* https://www.lua.org/pil/28.2.html
* -------------------------------------------------------------------------
* - a userdatum can have a metatable
* - Lua code cannot change the metatable of a userdatum
* - Create new array -> set metatable, get array -> check metatable
* - In Lua you register a new type i/t registry, using R[typename] = metatable
*    - index = the type name
*    - value = the value of the metatable

*/

typedef struct NumArray {
  int size;
  double values[1];  /* variable part */
} NumArray;

// new auxiliary func to check validity of userdatum as an NumArray
static NumArray *
checkarray (lua_State *L)
{
    void *ud = luaL_checkudata(L, 1, "ex03.array");
    fprintf(stderr, "hmm ud @ %p\n", ud);
    luaL_argcheck(L, ud != NULL, 1, "`array' expected");
    return (NumArray *)ud;
}

// refactoring code from both setarray, getarray (arg checking)
static double *
getelem (lua_State *L)
{
    NumArray *a = checkarray(L);
    int index = luaL_checkinteger(L, 2);

    luaL_argcheck(L, 1 <= index && index <= a->size, 2,
            "index out of range");

    /* return element address */
    return &a->values[index - 1];
}

static int
newarray (lua_State *L)
{
  int n = luaL_checkinteger(L, 1);
  size_t nbytes = sizeof(NumArray) + (n - 1)*sizeof(double);

  NumArray *a = (NumArray *)lua_newuserdata(L, nbytes);
  // [.., n, {ud}]

  luaL_getmetatable(L, "ex03.array");
  // [.., n, {ud}, {M}]  gets M via name & pushed onto stack
  lua_setmetatable(L, -2);

  // [.., n, {ud}], M is now ud's metatable

  a->size = n;

  return 1;  /* new userdatum is already on the stack */
}

static int
setarray (lua_State *L)
{
    double newval = luaL_checknumber(L, 3);
    *getelem(L) = newval;

    return 0;
}

static int
getarray (lua_State *L)
{
  /* NumArray *a = (NumArray *)lua_touserdata(L, 1); */
  /* int index = luaL_checkinteger(L, 2); */
  /* luaL_argcheck(L, a != NULL, 1, "`array' expected"); */
  /* luaL_argcheck(L, 1 <= index && index <= a->size, 2, */
  /*     "index out of range"); */
  /* lua_pushnumber(L, a->values[index-1]); */

    fprintf(stderr, "getarray!\n"); // doesn't print anything ?

    lua_pushnumber(L, *getelem(L));
    return 1;
}

static int
getsize (lua_State *L)
{
  // NumArray *a = (NumArray *)lua_touserdata(L, 1);
  // luaL_argcheck(L, a != NULL, 1, "`array' expected");
  NumArray *a = checkarray(L);
  lua_pushnumber(L, a->size);

  return 1;
}


//  REGISTER LIBRARY


// create list of (lua_name, c-func)'s
static const struct luaL_Reg funcs [] = {
  {"new", newarray},
  {"set", setarray},
  {"get", getarray},
  {"size", getsize},
  {NULL, NULL}        // sentinel
};


//luaopen_<name_as_required>
int luaopen_ex03 (lua_State *L)
{
  // the following replaces Lua 5.1's 'luaL_openlib(L, "array", arraylib, 0);'
  // Instead of creating a global table, the will make require return an
  // anonymous table, which the user must assign to a variable of its own.
  //
  // In Lua you would do: arr = require("ex03"); and use it: arr.new(..)
  luaL_newmetatable(L, "ex03.array");  // creates, pushes & registers
  lua_newtable(L);                     // [.., {M},{f's}]
  luaL_newlib(L, funcs);
  return 1;
}
