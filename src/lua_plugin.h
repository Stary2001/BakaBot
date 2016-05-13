#include <string>
#include "lua.hpp"
#include "command.h"

class LuaPlugin : public Plugin
{
public:
	LuaPlugin(std::string filename);
	~LuaPlugin();
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();

    lua_State *state;
    Bot *_bot;
    std::string _name;
    int plugin_ref;
};

class LuaCommand : public CommandBase
{
public:
	LuaCommand(lua_State *s, std::string n, int r);
	virtual void run(Bot *b, CommandInfo *i);
	virtual ~LuaCommand();
	virtual std::string name();

	void setup_state();
private:
	lua_State *state;
	std::string _name;
	int cmd_ref;
};
