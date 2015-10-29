#include <string>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netdb.h>
#include "connection.h"
#include <iostream>
#include <errno.h>
#include <unistd.h>

Connection::Connection() {}

Connection::Connection(int fd)
{
	m_fd = fd;
}

Connection::Connection(std::string host, unsigned short port)
{
    struct addrinfo hints = {0};
    struct addrinfo *list = NULL;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int r = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &list);
    if(r > 0)
    {
        std::cerr << "getaddrinfo failed" << std::endl;
    }

    while(list)
    {
        m_fd = socket(list->ai_family, list->ai_socktype, list->ai_protocol);
        if(m_fd != -1)
        {
            if(connect(m_fd, list->ai_addr, list->ai_addrlen) == 0)
            {
                std::cerr << "connected to " << list->ai_family << std::endl;
                // we connected!
                return;
            }
            else
            {
                close(m_fd);
            }
        }

        list = list->ai_next;
    }

    throw SocketException();
}

Connection::~Connection()
{
	close(m_fd);
}

void Connection::handle(uint32_t events)
{
	// todo: handle default connections
	std::cerr << "! " << events << std::endl;
}

int Connection::read(void* buf, int len)
{
	int l = ::read(m_fd, buf, len);
	if(l == 0)
	{
		std::cerr << "empty socket >:|" << std::endl;
	}
	return l;
}

int Connection::write(const char* c, size_t l)
{
	int sent = send(m_fd, c, l, 0);
	if(sent == 0)
	{
		std::cerr << "empty socket >:|" << std::endl;
	}
	return sent;
}


ConnectionDispatcher::ConnectionDispatcher()
{
	m_epoll_fd = epoll_create(1);
	if(m_epoll_fd == -1)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}
}

void ConnectionDispatcher::add(Connection *conn)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP;
	ev.data.ptr = conn;

	if(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, conn->m_fd, &ev) != 0)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}
	conn->dispatcher = this;
}

void ConnectionDispatcher::remove(Connection *conn)
{
	if(epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, conn->m_fd, NULL) != 0)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}
	conn->dispatcher = NULL;
}

#define EPOLL_EVENTS 16
void ConnectionDispatcher::handle()
{
	epoll_event events[EPOLL_EVENTS];

	int i = 0;
	int num = epoll_wait(m_epoll_fd, events, EPOLL_EVENTS, -1);
	if(num == -1)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}

	for(; i < num; i++)
	{
		((Connection*) events[i].data.ptr)->handle(events[i].events);
	}
}
