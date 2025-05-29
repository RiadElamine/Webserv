

#include "ConfigFile.hpp"


int main()
{
    ConfigFile c;
    try
    {
        c.parse("env");
        std::vector<ServerConfig>  &servers = c.servers;
        for (size_t i = 0; i < servers.size(); ++i) {
                std::cout << "Server " << i << ":\n";
                std::cout << "  host: " << servers[i].host << std::endl;
                std::cout << "  port: " << servers[i].port << std::endl;
                std::cout << "  client_max_body_size: " << servers[i].client_max_body_size << std::endl;
                std::cout << "  global_root: " << servers[i].global_root << std::endl;

                for (size_t j = 0; j < servers[i].locations.size(); ++j) {
                    const Location& loc = servers[i].locations[j];
                    std::cout << "  Location " << j << ":\n";
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
            }

    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (1);
    }
    
    std::cout << "every thing is good" << std::endl;
}

