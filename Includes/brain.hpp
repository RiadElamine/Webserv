#pragma once

#include "./Server/ConfigFile.hpp"

class HttpRequest;
class Cgi;
class Response; 

struct KqueueContext {
    std::vector<struct kevent>  evlist;
    struct kevent               event;
    int                         kq;
    std::map<int, HttpRequest *>  clientRequests;
    std::map<int, Cgi *>          clientCgiProcesses;
    std::map<int, Response *>     clientResponses;
};

class brain
{
    protected:
        Location *currentLocation;
        ServerConfig *currentServer;
        std::string headers_buffer_CGI;
        std::string path;
        std::string filename;
        int client_fd;
        std::map<std::string, std::string> params;
        KqueueContext *Context;
    public :
        bool is_cgi;
        ConnectionState state_of_connection;
        brain();
        brain( ServerConfig *server, int client_fd, KqueueContext *Context);
};