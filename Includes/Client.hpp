
#pragma once

#include "./Request/HttpRequest.hpp"
#include "./CGI/Cgi.hpp"
#include "./Response/response.hpp"

class Client : public HttpRequest, public Cgi, public Response
{
    public:
        Client(int fd, KqueueContext *context, ServerConfig *server);
        ~Client() {};
};