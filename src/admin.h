class AdminPlugin : public Plugin
{
public:
    virtual void init(PluginHost *h);
    virtual void deinit(PluginHost *h);
    virtual std::string name();
private:
	Bot *bot;
	bool load(Event *e);
	bool unload(Event *e);
	bool permissions(Event *e);
	bool group(Event *e);
};