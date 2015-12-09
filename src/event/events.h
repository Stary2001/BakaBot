#pragma once
#include "ircconnection.h"
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

class EventSink
{
public:
	EventSink();
	void handle_event();
	void queue_event(Event *ev);
	void add_handler(std::string type, EventHandler e);
private:
	std::queue<Event*> events;
	std::mutex events_mutex;
	std::condition_variable events_avail_cv;
	bool events_avail = false;

	std::map<std::string, std::vector<EventHandler>> handlers;
};

class IRCEvent : public Event
{
public:
	IRCEvent(std::string type, User u);
	std::string type;
    User sender;
};

class RawIRCEvent : public IRCEvent
{
public:
	RawIRCEvent(std::string type, User u, std::vector<std::string> p);
	std::vector<std::string> params;
};

class IRCConnectedEvent : public IRCEvent
{
public:
	IRCConnectedEvent(User s);
};

class IRCInviteEvent : public IRCEvent
{
public:
	IRCInviteEvent(User s, std::string target, std::string channel);
	std::string target;
	std::string channel;
};

class IRCMessageEvent : public IRCEvent
{
public:
	IRCMessageEvent(User s, std::string target, std::string message);
    std::string target;
    std::string message;
};

