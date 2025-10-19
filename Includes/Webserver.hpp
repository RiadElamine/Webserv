#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

// #include "../Includes/HttpRequest.hpp"
#include "response.hpp"
#include "Cgi_handler.hpp"

class WebServer {
    public:
        WebServer(std::vector<ServerConfig>  &servers);
        ~WebServer();
        void startServer();
    private:
        KqueueContext              Context;
        std::map<int, ServerConfig *> listeners;

        // Methods
        void    registerEvents();
        void    _handleAccept();
        void    _handleReadable();
        void     _handleWritable();
        void    _closeConnection();
        void    setNonBlocking(int fd);


        void handleReceiveEvent();
        void handleTimeoutEvent();
};

#endif