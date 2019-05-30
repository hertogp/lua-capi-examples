static void
stackDump (lua_State *L, const char *title)
{
    // print out the scalar value or type of args on the stack 
    int i;
    int top = lua_gettop(L); /* depth of the stack */
    printf("%20s : [", title);
    for (i = 1; i <= top; i++) { /* repeat for each level */
        int t = lua_type(L, i);
        switch (t)
        {
            case LUA_TNIL:
                {
                    printf("nil");
                    break;
                }
            case LUA_TBOOLEAN:
                {
                    printf(lua_toboolean(L, i) ? "true" : "false");
                    break;
                }
            case LUA_TLIGHTUSERDATA:
                {
                    printf("lud@%p", lua_touserdata(L, i));
                    break;
                }
            case LUA_TNUMBER:
                {
                    printf("%g", lua_tonumber(L, i));
                    break;
                }
            case LUA_TSTRING:
                {
                    printf("'%s'", lua_tostring(L, i));
                    break;
                }
            case LUA_TTABLE:
                {
                    printf("{}");
                    break;
                }
            case LUA_TFUNCTION:
                {
                    printf("f@%p", lua_touserdata(L, i));
                    break;
                }
            case LUA_TUSERDATA:
                {
                    printf("ud(%p)", lua_touserdata(L, i));
                    break;
                }
            case LUA_TTHREAD:
                {
                    printf("Thrd(%p)", lua_touserdata(L, i));
                    break;
                }
            default:
                { // other values
                    printf("%s", lua_typename(L, t));
                    break;
                }
        }
        if (i<top) printf(" "); /* put a separator */
    }
    printf("]\n"); /* end the listing */
}
