#include "../../Includes/Server/Webserver.hpp"
#include "../../Includes/utils.hpp"


WebServer::WebServer(std::vector<ServerConfig>  &servers) {
    for (size_t i = 0; i < servers.size(); ++i) {
        for (std::set<std::pair<std::string, uint16_t> >::iterator it = servers[i].listens.begin(); it != servers[i].listens.end(); ++it) {
            //
            std::stringstream   portStr;
            struct              addrinfo hints;
            struct              addrinfo *servinfo;

            memset(&hints, 0, sizeof(hints));
            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;

            std::string host = it->first;
            portStr << it->second; 
            int ad = getaddrinfo(host.c_str(), portStr.str().c_str(), &hints, &servinfo);
            if (ad != 0) {
                std::cerr << "getaddrinfo: ";
                throw std::runtime_error(gai_strerror(ad));
            }
            //
            struct addrinfo p = *servinfo;
            int fd = socket(p.ai_family, p.ai_socktype, p.ai_protocol);
            if (fd < 0) {
                throw std::runtime_error("Failed to create socket");
            }
            //
            int yes = 1;
            if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
                throw std::runtime_error("Failed to set socket options");
            }
            //
            if (bind(fd, p.ai_addr, p.ai_addrlen) == -1) {
                throw std::runtime_error("Failed to bind socket");
            }
            //
            if (listen(fd, SOMAXCONN) == -1) {
                throw std::runtime_error("Failed to listen on socket");
            }
            //
            listeners.insert(std::make_pair(fd, &servers[i]));
            freeaddrinfo(servinfo);
        }
    }
}

WebServer::~WebServer() {
    for (std::map<int, ServerConfig *>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
        close(it->first);
    }
    listeners.clear();
}

void _addEvent(std::vector<struct kevent> &events,
                          uintptr_t ident, int16_t filter, uint16_t flags,
                          uint32_t fflags, intptr_t data, void* udata) {
    struct kevent ev;
    EV_SET(&ev, ident, filter, flags, fflags, data, udata);
    events.push_back(ev);
}

void WebServer::registerEvents() {
    // Prepare events for listener sockets (server sockets)
    std::vector<struct kevent> listenerEvents;
    listenerEvents.reserve(listeners.size());
    // Register listener sockets for read events
    for (std::map<int, ServerConfig*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
        int fd = it->first;
        _addEvent(listenerEvents, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void *)server_event);
    }
    // Register all listener events with kqueue
    if (kevent(Context.kq, listenerEvents.data(), listenerEvents.size(), NULL, 0, NULL) == -1) {
        throw std::runtime_error("Failed to register listener events");
    }
    // resize evlist to hold listener events
    Context.evlist.resize(listeners.size());
}

void setNonBlocking(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1)
    {
        close(fd);
        throw std::runtime_error("fcntl(F_SETFL) failed");
    }
}

void WebServer::_handleAccept() {
    // Accept new connection
    int client_fd = accept(Context.event.ident, NULL, NULL);
    if (client_fd == -1)
        throw std::runtime_error("Failed to accept connection");
    // Set socket options
    int yes = 1;
    if (setsockopt(client_fd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes)) == -1)
    {
        close(client_fd);
        throw std::runtime_error("setsockopt(SO_NOSIGPIPE) failed");
    }
    setNonBlocking(client_fd);
    // Prepare events for the new client
        std::vector<struct kevent> ev;
        _addEvent(ev, client_fd, EVFILT_READ,  EV_ADD | EV_ENABLE, 0, 0, (void *)client_event);
        _addEvent(ev, client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, (void *)client_event);
        _addEvent(ev, client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timeout, (void *)client_event);
        // Register the new client events
        if (kevent(Context.kq, ev.data(), ev.size(), NULL, 0, NULL) == -1) {
            close(client_fd);
            throw std::runtime_error("Failed to register client events");
        }

    // Initialize HttpRequest for the new client
    Client *newClient = new Client(client_fd, &Context, listeners[Context.event.ident]);
    Context.clientResponses[client_fd] = static_cast<Response *>(newClient);
    Context.clientRequests[client_fd] = static_cast<HttpRequest *>(newClient);
    clients[client_fd] = newClient;
    std::cout << "Client connected: " << client_fd << std::endl;
}


bool isCgiRequest(Client &client) {
    // convete client to HttpRequest
    HttpRequest &request  = static_cast<HttpRequest&>(client);
    Response &response = static_cast<Response&>(client);
    Location *currentLocation;

    ServerConfig *currentServer = request.getServer();
    currentLocation = getCurrentLocation(request.getPath(), currentServer);
    response.setCurrentLocation(currentLocation);
    response.setPath(buildPath(currentLocation->URI, request.getPath(), currentLocation->root));

    client.is_cgi = isCGI(response.getPath(), currentLocation);
    return client.is_cgi;
}

void WebServer::_handleReadable() {
    // Read data from client
    int client_fd = Context.event.ident;
    char buffer[1024];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {
        clients[Context.event.ident]->state_of_connection = DISCONNECTED;
        return;
    }
    
    if (Context.clientRequests[client_fd]->parse_request(buffer, n))
    {
        // request fully received
        // check if it's a cgi request or not
        if (isCgiRequest(*(clients[client_fd])))
        {
            // if i have cgi to execute, i will do it here
            Context.clientCgiProcesses.insert(
                std::pair<int, Cgi *>(client_fd, &static_cast<Cgi&>(*(clients[client_fd])))
            );

            std::cout << "CGI request detected for client: " << client_fd << std::endl;
           
            Context.clientCgiProcesses.at(client_fd)->executeCgi();
            return;
        }
        else 
        {
            // NO CGI, so we prepare to send response
            std::vector<struct kevent> ev;
            _addEvent(ev, client_fd, EVFILT_READ,  EV_DISABLE, 0, 0, (void*)client_event);
            _addEvent(ev, client_fd, EVFILT_WRITE, EV_ENABLE, 0, 0, (void*)client_event);
            if (kevent(Context.kq, ev.data(), ev.size(), NULL, 0, NULL) == -1) {
                // remember to close the connection and clean up
                throw std::runtime_error("Failed to modify client events");
            }
        }
        std::cout << "Request received from client: " << client_fd << std::endl;
    }
    // Reset the timer
    struct kevent ev;
    EV_SET(&ev, client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timeout, (void*)client_event);
    if (kevent(Context.kq, &ev, 1, NULL, 0, NULL) == -1) {
        // remember to close the connection and clean up
        throw std::runtime_error("Failed to reset timer");
    }
}
// here is my functions

void WebServer::_handleWritable() {
    Response &response = (*Context.clientResponses[Context.event.ident]);
    HttpRequest &request = (*Context.clientRequests[Context.event.ident]);

    getDataFromRequest(request, response);
    
    // response.execute_method();

    std::string message("");
    if (!response.is_header_sent()) {
        message = response.getHeader();
        std::cout << "header: " << message;
        response.set_header_sent(true);
    } else {
        message = response.Read_chunks(100);
    }
    std::cout << "message: " << message ;
    if (!message.empty()) {
        size_t n = send(Context.event.ident, message.c_str(), message.length(), 0);
        if (n <= 0) {
            clients[Context.event.ident]->state_of_connection = DISCONNECTED;
            return ;
        }
    }

//    if data send successfully, we close the connection
    else
        clients[Context.event.ident]->state_of_connection = DISCONNECTED;
}

void WebServer::handleReceiveEvent()
{
    if (Context.event.udata == (void*)server_event)
    {
        // new client connection
        _handleAccept();
    }
    else if (Context.event.udata == (void*)client_event)
    {
        // client read event
        _handleReadable();
    }
   else
   {
       // cgi read event
       Cgi *cgiClient = static_cast<Cgi*>(Context.event.udata);
       cgiClient->_readCgiOutput();

   }
}

void WebServer::handleTimeoutEvent()
{
   if (Context.event.udata == (void*)client_event)
   {
       std::cout << "Connection timed out: " << Context.event.ident << std::endl;
       clients[Context.event.ident]->state_of_connection = DISCONNECTED;
   }
   else
   {
       // CGI timeout event
       std::cout << "CGI process timed out for client: " << std::endl;
       Cgi *e = static_cast<Cgi*>(Context.event.udata);
       e->finalizeCgiProcess(Gateway_Timeout);
    }
}

void WebServer::_closeConnection() {
    
    std::vector<struct kevent> ev;
    int fd_client = Context.event.ident;
    // remove client events from kqueue
    _addEvent(ev, fd_client, EVFILT_READ,  EV_DELETE, 0, 0, NULL);
    _addEvent(ev, fd_client, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    _addEvent(ev, fd_client, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    if (kevent(Context.kq, ev.data(), ev.size(), NULL, 0, NULL) == -1) {
        // remember to close the connection and clean up
        throw std::runtime_error("Failed to remove client events");
    }
    // if there is a cgi process associated with this client, remove its events too
    std::map<int, Cgi *>::iterator it = Context.clientCgiProcesses.find(fd_client);
    if (it != Context.clientCgiProcesses.end())
    {
        // remove cgi Process from map
        Context.clientCgiProcesses.erase(it);
    }
    // remove client request from map
    Context.clientRequests.erase(fd_client);
    // remove client response from map
    Context.clientResponses.erase(fd_client);
    // close client socket
    shutdown(fd_client, SHUT_WR);
    std::cout << "Shutting down connection: " << fd_client << std::endl;
    // close(fd_client);
    std::cout << "Connection closed: " << fd_client << std::endl;
}

void WebServer::startServer() {

    Context.kq = kqueue();
    if (Context.kq == -1) throw std::runtime_error("Failed to create kqueue");

    registerEvents();

    while (true) {
        int nev = kevent(Context.kq, NULL, 0, Context.evlist.data(), Context.evlist.size(), NULL);
        if (nev == -1)
        {
            throw std::runtime_error("kevent error");
        }
        if (nev == (int)Context.evlist.size())
        {
            Context.evlist.resize(Context.evlist.size() * 2);
        }

        for (int i = 0; i < nev; ++i) {
            Context.event = Context.evlist[i];

            if (Context.event.filter == EVFILT_READ)
            {
                handleReceiveEvent();
            } 
            else if (Context.event.filter == EVFILT_WRITE)
            {
                _handleWritable();
            }
            else if (Context.event.filter == EVFILT_TIMER)
            {
                handleTimeoutEvent();
            }
            else if (Context.event.filter == EVFILT_PROC && (Context.event.fflags & NOTE_EXIT))
            {
                // CGI process exited
                Cgi* cgiClient = static_cast<Cgi*>(Context.event.udata);
                cgiClient->handleCgiCompletion();
            }
            if (Context.event.udata == (void *)client_event &&
                    clients[Context.event.ident]->state_of_connection == DISCONNECTED)
            {
                _closeConnection();
            }
        }

    }
}
