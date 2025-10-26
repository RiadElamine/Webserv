
#include "../Includes/Client.hpp"

Client::Client(int fd, KqueueContext *context, ServerConfig *server)
    : brain(server, fd, context), HttpRequest(), Cgi(), Response()
{
    std::cout << "new Client Connection: " << client_fd << std::endl;
}
