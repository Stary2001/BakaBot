#include "connection.h"
#include <iostream>

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

template <class ConnType> Listener<ConnType>::Listener(std::string bind, unsigned short port)
{
        m_fd = create_listening_socket(bind, port);
}

template <class ConnType> int Listener<ConnType>::create_listening_socket(std::string addr, unsigned short port)
{
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        std::cout << "creating socket for " << port << std::endl;
        if(fd != -1)
        {
                struct sockaddr_in saddr;
                saddr.sin_family = AF_INET;
                saddr.sin_port = htons(port);

                inet_aton(addr.c_str(), &saddr.sin_addr);

                if(bind(fd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) != 0)
                {
                        // ono?
                        return -1;
                }
                if(listen(fd, 10) != 0)
                {
                        return -1;
                }
        }

        return fd;
}

template <class ConnType> void Listener<ConnType>::handle(uint32_t events)
{
	if(events & EPOLLIN)
	{
		sockaddr_in addr;
		socklen_t addr_len = sizeof(sockaddr_in);
		int client = accept(m_fd, (sockaddr*)&addr, &addr_len);
		dispatcher->add(new ConnType(client));
	}
}
