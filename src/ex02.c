// file ex01.c
// gcc -Isrc -undefined -shared -fPIC -o ex01.so src/ex01.c

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

/* https://www.lua.org/pil/28.1.html
** -------------------------------------------------------------------------
** Array in C example, complete with a glaring security hole:
** see -> https://www.lua.org/pil/28.2.html and ex03
*/

typedef struct NumArray {
  int size;
  double values[1];  /* variable part */
} NumArray;


static int
newarray (lua_State *L)
{
  int n = luaL_checkinteger(L, 1);
  size_t nbytes = sizeof(NumArray) + (n - 1)*sizeof(double);

  NumArray *a = (NumArray *)lua_newuserdata(L, nbytes);
  a->size = n;

  return 1;  /* new userdatum is already on the stack */
}

static int
setarray (lua_State *L)
{
  NumArray *a = (NumArray *)lua_touserdata(L, 1);
  int index = luaL_checkinteger(L, 2);
  double value = luaL_checknumber(L, 3);

  luaL_argcheck(L, a != NULL, 1, "`array' expected");
  luaL_argcheck(L, 1 <= index && index <= a->size, 2, "index out of range");
  a->values[index-1] = value;

  return 0;
}

static int
getarray (lua_State *L)
{
  NumArray *a = (NumArray *)lua_touserdata(L, 1);
  int index = luaL_checkinteger(L, 2);

  luaL_argcheck(L, a != NULL, 1, "`array' expected");

  luaL_argcheck(L, 1 <= index && index <= a->size, 2,
      "index out of range");

  lua_pushnumber(L, a->values[index-1]);
  return 1;
}

static int
getsize (lua_State *L)
{
  NumArray *a = (NumArray *)lua_touserdata(L, 1);

  luaL_argcheck(L, a != NULL, 1, "`array' expected");
  lua_pushnumber(L, a->size);

  return 1;
}


//library to be registered
static const struct luaL_Reg funcs [] = {
  {"new", newarray},
  {"set", setarray},
  {"get", getarray},
  {"size", getsize},
  {NULL, NULL}        // sentinel
};

int luaopen_array (lua_State *L) {
  // the following replaces Lua 5.1's 'luaL_openlib(L, "array", arraylib, 0);'
  // Instead of creating a global table, the will make require return an
  // anonymous table, which the user must assign to a variable of its own.
  //
  // In Lua you would do: arr = require("ex02"); and use it: arr.new(..)

  lua_newtable(L);             // [.., {}]
  luaL_setfuncs(L, funcs, 0);
  return 1;


  return 1;
}

//luaopen_<name_as_required>
int luaopen_ex02 (lua_State *L)
{
    luaL_newlib(L, funcs);
    return 1;
}
