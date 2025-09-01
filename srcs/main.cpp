

#include "../Includes/Webserver.hpp"


int main(int argc, char **argv)
{
    std::vector<ServerConfig>  *servers = NULL;
    // Parse the configuration file
    try
    {
        ConfigFile c(argc, argv);
        c.parse();
        servers = c.servers;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (1);
    }

    //display the parsed configuration
    // for (size_t i = 0; i < servers->size(); ++i) {
    //     std::cout << "Server " << i << ":\n";
    //     std::cout << "  server_name: " << (*servers)[i].server_name << std::endl;
    //     for (std::set<std::pair<std::string, uint16_t> >::iterator it = (*servers)[i].listens.begin(); it != (*servers)[i].listens.end(); ++it) {
    //         std::cout << "  Listen: " << it->first << ":" << it->second << std::endl;
    //     }
    //     std::cout << "  client_max_body_size: " << (*servers)[i].client_max_body_size << std::endl;
    //     std::cout << "  global_root: " << (*servers)[i].global_root << std::endl;

    //     for (size_t j = 0; j < (*servers)[i].locations.size(); ++j) {
    //         const Location& loc = (*servers)[i].locations[j];
    //         std::cout << "  Location " << j << ":\n";
    //         std::cout << "    URI: " << loc.URI << std::endl;
    //         std::cout << "    root: " << loc.root << std::endl;
    //         std::cout << "    index: " << loc.index << std::endl;
    //         std::cout << "    methods: ";
    //         for (size_t m = 0; m < loc.methods.size(); ++m) {
    //             std::cout << loc.methods[m] << " ";
    //         }
    //         std::cout << std::endl;
    //         std::cout << "    autoindex: " << (loc.autoindex ? "true" : "false") << std::endl;
    //         std::cout << "    upload_store: " << loc.upload_store << std::endl;
    //     }

    //     std::cout << " Error Pages : {" <<std::endl;
    //     for (std::map<int, std::string>::iterator m = (*servers)[i].error_pages.begin(); m != (*servers)[i].error_pages.end(); ++m) {
    //         std::cout << "      " << m->second << std::endl;
    //     }
    //     std::cout << " }" <<std::endl;
    // }
    try
    {
        WebServer webserver(*servers);
        std::cout << "server: waiting for connections..." << std::endl;
        webserver.startServer();
    }
    catch(const std::exception& e)
    {
        std::cerr << "WebServer error: ";
        std::cerr << e.what() << '\n';
    }
    
    free(servers);
    std::cout << "every thing is good" << std::endl;
}

