#pragma once

//
// This is a modified version of LunaFive, converted to store shared_ptr<T> rather than T* in userdata 
// The original LunaFive is here: http://lua-users.org/wiki/LunaFive
//

#include "lua.hpp"
#include <string.h> // For strlen
#include <sstream>

#ifndef NO_LUA_STACKDUMP
extern void stackDump(lua_State *L);
#endif

namespace Luna
{

using namespace std;

template < class T > class LunaShare {
  public:

    struct PropertyType {
	const char     *name;
	int             (T::*getter) (lua_State *);
	int             (T::*setter) (lua_State *);
    };

    struct FunctionType {
	const char     *name;
	int             (T::*func) (lua_State *);
    };

/*
  @ check
  Arguments:
    * L - Lua State
    * narg - Position to check

  Description:
    Retrieves a wrapped class from the arguments passed to the func, specified by narg (position).
    This func will raise an exception if the argument is not of the correct type.
*/
    static shared_ptr<T> check(lua_State * L, int narg)
    {
	shared_ptr<T> *obj = static_cast <shared_ptr<T> *>(luaL_checkudata(L, narg, T::className));
	if ( !obj )
		return nullptr; // lightcheck returns nullptr if not found.
	return *obj;		// pointer to T object
    }

/*
  @ lightcheck
  Arguments:
    * L - Lua State
    * narg - Position to check

  Description:
    Retrieves a wrapped class from the arguments passed to the func, specified by narg (position).
    This func will return nullptr if the argument is not of the correct type.  Useful for supporting
    multiple types of arguments passed to the func
*/ 
    static shared_ptr<T> *lightcheck(lua_State * L, int narg) 
    {
        shared_ptr<T> *obj = static_cast <shared_ptr<T> *>(luaL_testudata(L, narg, T::className));
        if ( !obj )
	        return nullptr; // lightcheck returns nullptr if not found.
        return *obj;		// pointer to T object
    }

/*
  @ Register
  Arguments:
    * L - Lua State
    * namespac - Namespace to load into

  Description:
    Registers your class with Lua.  Leave namespac "" if you want to load it into the global space.
*/
    // REGISTER CLASS AS A GLOBAL TABLE 
    static void Register(lua_State * L, const char *namespac = NULL ) {

		if ( namespac && strlen(namespac) )
		{
			lua_getglobal(L, namespac);
			if( lua_isnil(L,-1) ) // Create namespace if not present
			{
				lua_newtable(L);
				lua_pushvalue(L,-1); // Duplicate table pointer since setglobal pops the value
				lua_setglobal(L,namespac);
			}
			lua_pushcfunction(L, &LunaShare < T >::constructor);
			lua_setfield(L, -2, T::className);
			lua_pop(L, 1);
		} else {
			lua_pushcfunction(L, &LunaShare < T >::constructor);
			lua_setglobal(L, T::className);
		}
		
		luaL_newmetatable(L, T::className);
		int             metatable = lua_gettop(L);
		
		lua_pushstring(L, "__gc");
		lua_pushcfunction(L, &LunaShare < T >::gc_obj);
		lua_settable(L, metatable);
		
		lua_pushstring(L, "__tostring");
		lua_pushcfunction(L, &LunaShare < T >::to_string);
		lua_settable(L, metatable);

		lua_pushstring(L, "__index");
		lua_pushcfunction(L, &LunaShare < T >::property_getter);
		lua_settable(L, metatable);

		lua_pushstring(L, "__newindex");
		lua_pushcfunction(L, &LunaShare < T >::property_setter);
		lua_settable(L, metatable);
		
		for (int i = 0; T::properties[i].name; i++) { 				// Register some properties in it
			lua_pushstring(L, T::properties[i].name);				// Having some string associated with them
			lua_pushnumber(L, i); 									// And a number indexing which property it is
			lua_settable(L, metatable);
		}
		
		for (int i = 0; T::methods[i].name; i++) {
			lua_pushstring(L, T::methods[i].name); 					// Register some functions in it
			lua_pushnumber(L, i | ( 1 << 8 ) );						// Add a number indexing which func it is
			lua_settable(L, metatable);								//
		}
    }

/*
  @ constructor (internal)
  Arguments:
    * L - Lua State
*/
    static int constructor(lua_State * L)
    {
        void* a = lua_newuserdata(L, sizeof(shared_ptr<T>)); // Push value = userdata
        assert(a == lua_touserdata(L, -1));
        new (a) shared_ptr<T>(new T(L)); // not using make_shared because we need new() to allocate aligned T
	
        luaL_getmetatable(L, T::className); 		// Fetch global metatable T::classname
        lua_setmetatable(L, -2);

#ifndef NO_LUA_STACKDUMP
        if (!lua_isuserdata(L, -1))
        {
            cerr << "Serious error in Lua object construction." << endl;
            stackDump(L);
        }
#endif

        return 1;
    }

/*
  @ createNew
  Arguments:
    * L - Lua State
	T*	- Instance to push

  Description:
    Loads an instance of the class into the Lua stack, and provides you a pointer so you can modify it.
*/
    static void push(lua_State * L, shared_ptr<T> instance )
    {
        shared_ptr<T> *a = (shared_ptr<T> *) lua_newuserdata(L, sizeof(shared_ptr<T>)); // Create userdata
        new (a) shared_ptr<T>();
        *a = instance;
		
        luaL_getmetatable(L, T::className);
		
        lua_setmetatable(L, -2);
    }

/*
  @ property_getter (internal)
  Arguments:
    * L - Lua State
*/
    static int property_getter(lua_State * L)
	{
		lua_getmetatable(L, 1); // Look up the index of a name
		lua_pushvalue(L, 2);	// Push the name
		lua_rawget(L, -2);		// Get the index
		
		if (lua_isnumber(L, -1)) { // Check if we got a valid index
			
			int _index = (int)lua_tonumber(L, -1);
			
			shared_ptr<T> *obj = static_cast<shared_ptr<T> *>(lua_touserdata(L, 1));
			
			lua_pushvalue(L, 3);
			
			if( _index & ( 1 << 8 ) ) // A func
			{
				lua_pushnumber(L, _index ^ ( 1 << 8 ) ); // Push the right func index
				lua_pushlightuserdata(L, obj);
				lua_pushcclosure(L, &LunaShare < T >::function_dispatch, 2);
				return 1; // Return a func
			}
			
			lua_pop(L,2);    // Pop metatable and _index
			lua_remove(L,1); // Remove userdata
			lua_remove(L,1); // Remove [key]
			
			return ((*obj).get()->*(T::properties[_index].getter)) (L);
		}
		
		return 1;
    }

/*
  @ property_setter (internal)
  Arguments:
    * L - Lua State
*/
    static int property_setter(lua_State * L)
	{
		
		lua_getmetatable(L, 1); // Look up the index from name
		lua_pushvalue(L, 2);	//
		lua_rawget(L, -2);		//
		
		if ( lua_isnumber(L, -1) ) // Check if we got a valid index
		{
			
			int _index = (int)lua_tonumber(L, -1);
			
			shared_ptr<T> *obj = static_cast<shared_ptr<T> *>(lua_touserdata(L, 1));
			
			if( !obj || !*obj )
			{
				luaL_error( L , "Internal error, no object given!" );
				return 0;
			}
			
			if( _index >> 8 ) // Try to set a func
			{
				//char c[128];
                                std::stringstream s;
				//sprintf( c , "Trying to set the method [%s] of class [%s]" , (*obj)->T::methods[_index ^ ( 1 << 8 ) ].name , T::className );
                                s << "Trying to set the method [" << (*obj)->T::methods[_index ^ ( 1 << 8 ) ].name << "] of class [" << T::className << "]";
				luaL_error( L , s.str().c_str() );
				return 0;
			}
			
			lua_pop(L,2);    // Pop metatable and _index
			lua_remove(L,1); // Remove userdata
			lua_remove(L,1); // Remove [key]
			
			return ((*obj).get()->*(T::properties[_index].setter)) (L);
		}
		
		return 0;
    }

/*
  @ function_dispatch (internal)
  Arguments:
    * L - Lua State
*/
    static int function_dispatch(lua_State * L)
    {
        int i = (int) lua_tonumber(L, lua_upvalueindex(1));
        shared_ptr<T> *obj = static_cast < shared_ptr<T> * >(lua_touserdata(L, lua_upvalueindex(2)));
		
        return ((*obj).get()->*(T::methods[i].func)) (L);
    }

/*
  @ gc_obj (internal)
  Arguments:
    * L - Lua State
*/
        static int gc_obj(lua_State * L)
        {
            shared_ptr<T> *obj = static_cast < shared_ptr<T> * >(lua_touserdata(L, -1));
		
            if( obj && *obj ) (*obj)->~T();
		
            return 0;
        }
	
	static int to_string(lua_State* L)
	{
	    shared_ptr<T> *obj = static_cast< shared_ptr<T> * >(lua_touserdata(L, -1));
		
	    if( obj )
		    lua_pushfstring(L, "%s (%p)", T::className, (void*)(*obj).get());
	    else
		    lua_pushstring(L,"Empty object");
		
	    return 1;
	}
};

}
