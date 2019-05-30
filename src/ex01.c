// file ex01.c

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <math.h>

static int
c_swap (lua_State *L)
{
    //check and fetch the arguments
    double arg1 = luaL_checknumber (L, 1);
    double arg2 = luaL_checknumber (L, 2);

    //push the results
    lua_pushnumber(L, arg2);
    lua_pushnumber(L, arg1);

    //return number of results
    return 2;
}

static int
c_sin (lua_State *L)
{
    double arg = luaL_checknumber (L, 1);
    lua_pushnumber(L, sin(arg));
    return 1;
}

//library to be registered
static const struct luaL_Reg funcs [] = {
      {"c_swap", c_swap},
      {"c_sin", c_sin},
      {NULL, NULL}        // sentinel */
};

//luaopen_<name_as_required>
int luaopen_ex01 (lua_State *L);
int luaopen_ex01 (lua_State *L)
{
    luaL_newlib(L, funcs);
    return 1;
}
