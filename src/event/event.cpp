#include "events.h"
#include <algorithm> // for tolower
#include <iostream>

EventSink::EventSink()
{
}

void EventSink::add_handler(std::string type, std::string id, EventHandler e)
{
	std::transform(type.begin(), type.end(), type.begin(), tolower);

	if(handlers.find(type) == handlers.end())
	{
		handlers[type] = std::vector<std::pair<std::string, EventHandler>>();
	}

	handlers[type].push_back(std::pair<std::string,EventHandler>(id, e));
}

void EventSink::remove_handler(std::string type, std::string id)
{
	auto it = handlers.find(type);
	if(it != handlers.end())
	{
		auto it2 = it->second.begin();

		for(; it2 != it->second.end(); it2++)
		{
			if(it2->first == id)
			{
				break;
			}
		}

		if(it2 != it->second.end())
		{
			it->second.erase(it2);
		}
	}
}

void EventSink::queue_event(Event *e)
{
	std::unique_lock<std::mutex> lk(events_mutex);
	std::cout << "queued event " << e->type << std::endl;
	events_avail = true;
	events.push(e);
	events_avail_cv.notify_one();
}

void EventSink::handle_event()
{
	Event *e = NULL;
	{
		std::unique_lock<std::mutex> lk(events_mutex);
	    events_avail_cv.wait(lk, [this]{return events_avail;});

		e = events.front();
		events.pop();
		if(events.size() == 0)
		{
			events_avail = false;
		}

		lk.unlock();
	}

	if(e == NULL) { return; }

	if(handlers.find(e->type) != handlers.end() && handlers[e->type].size() != 0)
	{
		for(std::pair<std::string, EventHandler> kv : handlers[e->type])
		{
			EventHandler ev = kv.second;
			if(ev(e))
			{
				break;
			}
		}
	}

	delete e;
}

Event::Event(std::string t) : type(t)
{}

IRCEvent::IRCEvent(std::string t, IRCUser *u) : Event(t), sender(u)
{}

RawIRCEvent::RawIRCEvent(std::string n, IRCUser *u, std::vector<std::string> p): IRCEvent(n, u), params(p)
{}

IRCConnectedEvent::IRCConnectedEvent(IRCUser *s) : IRCEvent("irc/connected", s)
{}

IRCRegisteredEvent::IRCRegisteredEvent() : Event("irc/registered")
{}

IRCInviteEvent::IRCInviteEvent(IRCUser *s, std::string t, std::string c) : IRCEvent("irc/invite", s), target(t), channel(c)
{}

IRCMessageEvent::IRCMessageEvent(IRCUser *s, std::string t, std::string msg) : IRCEvent("irc/message", s), target(t), message(msg)
{}

IRCMessageEvent::IRCMessageEvent(std::string type, IRCUser *s, std::string t, std::string msg) : IRCEvent(type, s), target(t), message(msg)
{}
