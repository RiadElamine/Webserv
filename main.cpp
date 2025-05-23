

#include "ConfigFile.hpp"


int main()
{
    ConfigFile c;
    try
    {
        c.parse("env");
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return (1);
    }
    
    std::cout << "every thing is good" << std::endl;
}

