

#include "ConfigFile.hpp"


int main()
{
    ConfigFile c;
    try
    {
        c.parse("env");
        int i = 0;
        for (auto it = c.servers.begin(); it != c.servers.end(); ++it) {
            std::cout << (*it).server_name << std::endl;
            std::cout << (*(*it).locations.begin()).URI << std::endl;
            std::cout << (*(*it).error_pages.begin()).first << " "  << (*(*it).error_pages.begin()).second << std::endl;
              std::cout << ++i << std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (1);
    }
    
    std::cout << "every thing is good" << std::endl;
}

