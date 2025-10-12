#include "../Includes/Webserver.hpp"

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

void WebServer::setNonBlocking(int fd) {
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
    Context.clientRequests[client_fd] = HttpRequest();
    Context.clientRequests[client_fd].setServer(listeners[Context.event.ident]);
    std::cout << "Client connected: " << client_fd << std::endl;
}

void WebServer::_handleReadable() {
    // Read data from client
    int client_fd = Context.event.ident;
    char buffer[1024];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
    if (n <= 0)
    {
        Context.state_of_connection = DISCONNECTED;
        return;
    }
    
    if (Context.clientRequests[client_fd].parse_request(buffer, n))
    {
        // // request fully received
        // // check if it's a cgi request or not
        // if (isCgiRequest(Context.clientRequests[client_fd]))
        {
            // if i have cgi to execute, i will do it here
            // std::cout << "CGI request detected for client: " << client_fd << std::endl;
            // Context.clientCgiProcesses.insert(
            //     std::pair<int, Cgi>(client_fd, Cgi(Context, client_fd))
            // );
            // Context.clientCgiProcesses.at(client_fd).executeCgi();
        }
        // else 
        {
            // NO CGI, so we prepare to send response
            std::vector<struct kevent> ev;
            _addEvent(ev, client_fd, EVFILT_READ,  EV_DISABLE, 0, 0, NULL);
            _addEvent(ev, client_fd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
            if (kevent(Context.kq, ev.data(), ev.size(), NULL, 0, NULL) == -1) {
                // remember to close the connection and clean up
                throw std::runtime_error("Failed to modify client events");
            }
        }
        std::cout << "Request received from client: " << client_fd << std::endl;
    }
    // Reset the timer
    struct kevent ev;
    EV_SET(&ev, client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timeout, NULL);
    if (kevent(Context.kq, &ev, 1, NULL, 0, NULL) == -1) {
        // remember to close the connection and clean up
        throw std::runtime_error("Failed to reset timer");
    }
}

int WebServer::_handleWritable() {
    // if (clientRequests[client_fd].getMethod() == "POST")
    //     return DISCONNECTED;
    Response response;
    getDataFromRequest(Context.clientRequests[Context.event.ident], response);
    response.execute_method();
    std::string message = response.getResponse();
    ssize_t n = send(Context.event.ident, message.c_str(), message.length(), 0);
    if (n == -1)
        return DISCONNECTED;

    //if data send successfully, we close the connection
    // std::cout << "Response sent to client: " << client_fd << std::endl;

    return DISCONNECTED;
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
        Cgi &cgiClient = Context.clientCgiProcesses.at(Context.event.ident);
        cgiClient._readCgiOutput();
        if (Context.state_of_connection == DISCONNECTED)
        {
            // if cgi return DISCONNECTED that means error happened
            cgiClient.handleCgiFailure(-1, true, DISCONNECTED);
        }
    }
}

void WebServer::handleTimeoutEvent()
{
    if (Context.event.udata == (void*)client_event)
    {
        std::cout << "Connection timed out: " << Context.event.ident << std::endl;
        Context.state_of_connection = DISCONNECTED;
    }
    else
    {
        // CGI timeout event
        Cgi *e = static_cast<Cgi*>(Context.event.udata);
        e->handleCgiFailure(Gateway_Timeout, true, CONNECTED);
        std::cout << "CGI process timed out for client: " << e->getClientFd() << std::endl;
    }
}

void WebServer::handleCgiCompletion()
{
    Cgi* cgiClient = static_cast<Cgi*>(Context.event.udata);
    // reap the cgi process to avoid zombie
    if (waitpid(cgiClient->getCgiPid(), &cgiClient->getStatus(), 0) == -1)
    {
        perror("waitpid");
        throw std::runtime_error("Failed to reap CGI process");
    }
    // check status of cgi process
    int status = cgiClient->getStatus();
    if ((WIFEXITED(status) && WEXITSTATUS(status) != 0 )|| WIFSIGNALED(status)) {
        // CGI exited with error
        // send 502 Bad Gateway response to client
        cgiClient->handleCgiFailure(Bad_Gateway, false, CONNECTED);
        return;
    }
    // check if cgi stdout is Done
    if (cgiClient->isStdoutDone())
    {
        // CGI finished successfully, dont close client connection yet (we need to send response)
        // only remove cgi events from kqueue
        cgiClient->handleCgiFailure(-1, false, CONNECTED);
        // enable write event on client socket to send response
        struct kevent ev;
        EV_SET(&ev, cgiClient->getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
        if (kevent(Context.kq, &ev, 1, NULL, 0, NULL) == -1)
            throw std::runtime_error("Failed to enable write event on client socket");
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
    std::map<int, Cgi>::iterator it = Context.clientCgiProcesses.find(fd_client);
    if (it != Context.clientCgiProcesses.end())
    {
        // remove cgi Process from map
        Context.clientCgiProcesses.erase(it);
    }
    // remove client request from map
    Context.clientRequests.erase(fd_client);
    // close client socket
    shutdown(fd_client, SHUT_WR);
    std::cout << "Shutting down connection: " << fd_client << std::endl;
    // close(fd);
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
            Context.state_of_connection = CONNECTED;

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
                handleCgiCompletion();
            }
            if (Context.state_of_connection == DISCONNECTED)
            {
                _closeConnection();
            }
        }
    }
}
