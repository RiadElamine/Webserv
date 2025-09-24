#include "../Includes/Webserver.hpp"


int main(int argc, char **argv)
{
    ServerConfig  *server = NULL;
    // Parse the configuration file
    try
    {
        ConfigFile c(argc, argv);
        server = c.parse();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (1);
    }

    //display the parsed configuration
    for (std::set<std::pair<std::string, uint16_t> >::iterator it = server->listens.begin(); it != server->listens.end(); ++it) {
        std::cout << "Listen: " << it->first << ":" << it->second << std::endl;
    }
    std::cout << "client_max_body_size: " << server->client_max_body_size << std::endl;
    std::cout << "global_root: " << server->global_root << std::endl;

    for (size_t j = 0; j < server->locations.size(); ++j) {
        const Location& loc = server->locations[j];
        std::cout << " Location " << j << ":\n";
        std::cout << "    URI: " << loc.URI << std::endl;
        std::cout << "    root: " << loc.root << std::endl;
        std::cout << "    index: " << loc.index << std::endl;
        std::cout << "    methods: ";
        for (size_t m = 0; m < loc.methods.size(); ++m) {
            std::cout << loc.methods[m] << " ";
        }
        std::cout << std::endl;
        std::cout << "    autoindex: " << (loc.autoindex ? "true" : "false") << std::endl;
        std::cout << "    upload_store: " << loc.upload_store << std::endl;
    }

    std::cout << " Error Pages : {" <<std::endl;
    for (std::map<int, std::string>::iterator m = server->error_pages.begin(); m != server->error_pages.end(); ++m) {
        std::cout << "      " << m->second << std::endl;
    }
    std::cout << " }" <<std::endl;


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
    
    free(server);
}