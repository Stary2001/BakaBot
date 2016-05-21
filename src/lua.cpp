#include <string>
#include "lua.hpp"
//#include "lauxlib.h"

#include "command.h"
#include "lua_plugin.h"

LuaPlugin::LuaPlugin(std::string filename)
{
	_name = filename;

	state = luaL_newstate();
	luaL_openlibs(state);
	luaL_dofile(state, "setup.lua");

	lua_getglobal(state, "setup");

	if(lua_pcall(state, 0, 0, 0) != LUA_OK)
	{
		const char *c = lua_tostring(state, -1);
		std::string s = c;
		lua_pop(state, -1);
		throw CommandErrorException(filename, s);
	}

	luaL_loadfile(state, filename.c_str());

	if(lua_pcall(state, 0, 1, 0) != LUA_OK)
	{
		const char *c = lua_tostring(state, -1);
		std::string s = c;
		lua_pop(state, -1);
		throw CommandErrorException(filename, s);
	}

	if(lua_isnil(state, -1))
	{
		lua_pop(state, -1);
		throw CommandErrorException(filename, "did not provide plugin");
	}

	plugin_ref = luaL_ref(state, LUA_REGISTRYINDEX); // store the plugin in the registry.
}

LuaPlugin::~LuaPlugin()
{
	luaL_unref(state, LUA_REGISTRYINDEX, plugin_ref);
}

void safe_call(lua_State *state, std::string n, int nargs, int nret, int err)
{
	if(lua_pcall(state, nargs, nret, err) != LUA_OK)
	{
		const char *c = lua_tostring(state, -1);
		std::string s = c;
		lua_pop(state, -1);
		throw CommandErrorException(n, s);
	}
}

void get_table(lua_State *state, const char *s)
{
	lua_pushstring(state, s);
	lua_rawget(state, -2);
	lua_remove(state, -2); // -1 holds the thing.
}

extern "C" int info_msg(lua_State *l)
{
	luaL_checkstring(l, 2);

	void *a = luaL_checkudata(l, 1, "info");
	CommandInfo *i = (CommandInfo*) a;
	i->next->in.push_back(new StringData(lua_tostring(l, 2)));
	return 0;
}

extern "C" int register_command(lua_State *state)
{
	void *b_ = luaL_checkudata(state, 1, "bot");
	Bot *b = (Bot*)b_;

	const char *s = luaL_checkstring(state, 2);

	if(lua_isfunction(state, 3))
	{
		lua_pushvalue(state, 3);
		int cmd_ref = luaL_ref(state, LUA_REGISTRYINDEX);

		b->register_command(s, new LuaCommand(state, s, cmd_ref));
	}
	else
	{
		// error
	}

	return 0;
}

extern "C" void push_bot(lua_State *state, Bot *b)
{
	static luaL_Reg bot_reg[] = 
	{
		{"register_command", register_command},
		{NULL, NULL}
	};

	lua_pushlightuserdata(state, b);
	if(luaL_newmetatable(state, "bot"))
	{
		lua_pushstring(state, "__index");
		luaL_newlib(state, bot_reg);
		lua_settable(state, -3);
	}
	lua_setmetatable(state, -2);
}

extern "C" void push_command_info(lua_State *state, CommandInfo *i)
{
	static luaL_Reg info_reg[] = 
	{
		{"msg", info_msg},
		{NULL, NULL}
	};

	lua_pushlightuserdata(state, i);
	if(luaL_newmetatable(state, "info"))
	{
		lua_pushstring(state, "__index");
		luaL_newlib(state, info_reg);
		lua_settable(state, -3);
	}
	lua_setmetatable(state, -2);
}

void LuaPlugin::init(PluginHost *h)
{
	Bot *b = (Bot*)h;

	lua_rawgeti(state, LUA_REGISTRYINDEX, plugin_ref);
	get_table(state, "init");

	push_bot(state, b);

	if(lua_isnil(state, -1))
	{
		lua_pop(state, -1);
		throw CommandErrorException(_name, "did not provide plugin.init");
	}

	safe_call(state, _name, 1, 0, 0);
}

void LuaPlugin::deinit(PluginHost *h)
{
	Bot *b = (Bot*)h;

	lua_rawgeti(state, LUA_REGISTRYINDEX, plugin_ref);
	get_table(state, "deinit");

	if(lua_isnil(state, -1))
	{
		lua_pop(state, -1);
		throw CommandErrorException(_name, "did not provide plugin.deinit");
	}
}

LuaCommand::LuaCommand(lua_State *s, std::string n, int r) : CommandBase(CommandFlags::None)
{
	_name = n;
	state = s;
	cmd_ref = r;
}

LuaCommand::~LuaCommand()
{
	luaL_unref(state, LUA_REGISTRYINDEX, cmd_ref);
}

void LuaCommand::run(Bot *b, CommandInfo *i)
{
	/*if(b != _plugin->_bot)
	{
		throw CommandErrorException(_name, "NO");
	}*/

	lua_rawgeti(state, LUA_REGISTRYINDEX, cmd_ref);
	push_bot(state, b);
	push_command_info(state, i);
	safe_call(state, _name, 2, 0, 0);
}

std::string LuaPlugin::name()
{
	return _name;
}

std::string LuaCommand::name()
{
	return _name;
}