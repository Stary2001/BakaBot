#pragma once
#include <string>
#include <stdint.h>

class ConnectionDispatcher;
class Connection
{
	friend class ConnectionDispatcher;
public:
	Connection();
	~Connection();
	Connection(int fd);
	Connection(std::string host, unsigned short port);
	virtual void handle(uint32_t events);
	int read(void* buf, int len);
	int write(const char* , size_t);
	
	int m_fd;
protected:
	ConnectionDispatcher *dispatcher;
};

template <class ConnType> class Listener : public Connection
{
public:
	Listener(int fd);
	Listener(std::string bind_addr, unsigned short port);
	virtual void handle(uint32_t events);
private:
	int create_listening_socket(std::string host, unsigned short port);
};

class ConnectionDispatcher
{
public:
        ConnectionDispatcher();
        void add(Connection *conn);
        void remove(Connection *conn);
        void handle();
private:
        int m_epoll_fd;
};


class EpollException : std::exception
{};

class SocketException : std::exception
{};
