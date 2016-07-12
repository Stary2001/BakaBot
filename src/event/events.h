#pragma once
#include "irc/ircconnection.h"
#include "export.h"
#include <vector>
#include <map>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

class Event
{
public:
	Event(std::string type);
	std::string type;
};

typedef std::function<bool(Event*)> EventHandler;

PLUGINCLASS EventSink
{
public:
	EventSink();
	void handle_event();
	void queue_event(Event *ev);
	void add_handler(std::string type, std::string id, EventHandler e);
	void remove_handler(std::string type, std::string id);
protected:
	std::map<std::string, std::vector<std::pair<std::string, EventHandler>>> handlers;
private:
	std::queue<Event*> events;
	std::mutex events_mutex;
	std::condition_variable events_avail_cv;
	bool events_avail = false;
};

class IRCEvent : public Event
{
public:
	IRCEvent(std::string type, IRCUser *u);
	std::string type;
    IRCUser *sender;
};

class RawIRCEvent : public IRCEvent
{
public:
	RawIRCEvent(std::string type, IRCUser *u, std::vector<std::string> p);
	std::vector<std::string> params;
};

class IRCConnectedEvent : public IRCEvent
{
public:
	IRCConnectedEvent(IRCUser *s);
};

class IRCInviteEvent : public IRCEvent
{
public:
	IRCInviteEvent(IRCUser *s, std::string target, std::string channel);
	std::string target;
	std::string channel;
};

class IRCMessageEvent : public IRCEvent
{
public:
	IRCMessageEvent(std::string type, IRCUser *s, std::string t, std::string msg);
	IRCMessageEvent(IRCUser *s, std::string target, std::string message);
    std::string target;
    std::string message;
};

class IRCRegisteredEvent : public Event
{
public:
	IRCRegisteredEvent();
};
