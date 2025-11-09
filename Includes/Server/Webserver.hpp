#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include "../Client.hpp"

class WebServer {
    public:
        WebServer(std::vector<ServerConfig>  &servers);
        ~WebServer();
        void startServer();
        void setupServerSockets();
    private:
        KqueueContext              Context;
        std::map<int, ServerConfig *> listeners;
        std::map<int, Client *>       clients;
        std::vector<ServerConfig>     &Servers;

        // Methods
        void    registerEvents();
        void    _handleAccept();
        void    _handleReadable();
        void     _handleWritable();
        void    _closeConnection();

        void handleReceiveEvent();
        void handleTimeoutEvent();
};

#endif