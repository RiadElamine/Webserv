#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <vector>
#include <map>
#include <string>
#include <sys/event.h>
#include <fcntl.h>
#include "../Includes/ConfigFile.hpp"
#include "../Includes/HttpRequest.hpp"


#define timeout 10000 // 10 seconds

struct Listener {
    int fd;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    ServerConfig *hosts;
};

enum ConnectionState {
    CONNECTED,
    DISCONNECTED
};

class WebServer {
    public:
        WebServer(std::vector<ServerConfig>  &servers);
        ~WebServer();
        void startServer();

    private:
        WebServer(const WebServer&);
        WebServer& operator=(const WebServer&);
        std::vector<Listener> listeners;
        std::map<int, HttpRequest> clientRequests;
        void registerEvents();

        int kq;
        std::vector<struct kevent> listenerEvents;

        void _handleAccept(int listen_fd);
        int _handleReadable(int client_fd);
        int _handleWritable(int client_fd);
        void _closeConnection(int fd);

        void _addEvent(std::vector<struct kevent> &events, uintptr_t ident, int16_t filter, uint16_t flags,
                          uint32_t fflags, intptr_t data, void* udata);
        void setNonBlocking(int fd);
};

#endif