
#include "../Includes/brain.hpp"


// default constructor
brain::brain() : currentLocation(NULL), currentServer(NULL), client_fd(-1), Context(NULL)
{
    is_cgi = false;
    state_of_connection = CONNECTED;
}

// parameterized constructor
brain::brain( ServerConfig *server, int client_fd, KqueueContext *Context) 
    : currentLocation(NULL), currentServer(server), client_fd(client_fd), Context(Context)
{
    is_cgi = false;
    state_of_connection = CONNECTED;
}

// destructor
brain::~brain()
{
    // free resources if needed
    delete currentLocation;
}