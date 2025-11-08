#include "../Includes/Server/Webserver.hpp"


int main(int argc, char **argv)
{
    std::vector<ServerConfig>  *server = NULL;
    // Parse the configuration file
    try
    {
        ConfigFile c(argc, argv);
        server = c.parse();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        if (server)
            delete server;
        return (1);
    }

    // Start the web server
    try
    {
        WebServer webserver(*server);
        std::cout << "server: waiting for connections..." << std::endl;
        webserver.startServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << "WebServer error: ";
        std::cerr << e.what() << '\n';

    }
    std::cout << "server: shutting down..." << std::endl;
    delete server;
}

