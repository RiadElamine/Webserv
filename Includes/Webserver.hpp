#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <map>
#include <string>
#include <sys/event.h>
#include "../Includes/ConfigFile.hpp"


#define timeout 10000

struct Listener {
    int fd;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    ServerConfig *hosts;
};

class WebServer {
    public:
        WebServer(std::vector<ServerConfig>  &servers);
        ~WebServer();
        void sartServer();

    private:
        WebServer(const WebServer&);
        WebServer& operator=(const WebServer&);
        std::vector<Listener> listeners;
        void registerEvents();

        int kq;
        std::vector<struct kevent> events;

        void    _handleAccept(int listen_fd);
        void    _handleReadable(int client_fd);
        void    _handleWritable(int client_fd);
        void    _closeConnection(int fd);


        void _addEvent(uintptr_t ident, int16_t filter, uint16_t flags,
                          uint32_t fflags, intptr_t data, void* udata);
};

#endif