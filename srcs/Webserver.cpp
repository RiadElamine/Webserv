#include "../Includes/Webserver.hpp"

WebServer::WebServer(ServerConfig  &servers) {
    for (std::set<std::pair<std::string, uint16_t> >::iterator it = servers.listens.begin(); it != servers.listens.end(); ++it) {
        Listener listener;

        listener.hosts = &servers; 

        memset(&listener.hints, 0, sizeof(listener.hints));
        listener.hints.ai_family = AF_INET;
        listener.hints.ai_socktype = SOCK_STREAM;

        std::string host = it->first;
        std::stringstream portStr;
        portStr << it->second; 
        int ad = getaddrinfo(host.c_str(), portStr.str().c_str(), &listener.hints, &listener.servinfo);
        if (ad != 0) {
            std::cerr << "getaddrinfo: ";
            throw std::runtime_error(gai_strerror(ad));
        }
        
        struct addrinfo p = *listener.servinfo;
        
        listener.fd = socket(p.ai_family, p.ai_socktype, p.ai_protocol);
        if (listener.fd < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        int yes = 1;

        if (setsockopt(listener.fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes))) {
            throw std::runtime_error("Failed to set socket options");
        }

        if (bind(listener.fd, p.ai_addr, p.ai_addrlen) == -1) {
            throw std::runtime_error("Failed to bind socket");
        }

        if (listen(listener.fd, SOMAXCONN) == -1) {
            throw std::runtime_error("Failed to listen on socket");
        }

        listeners.push_back(listener);
    }
}

WebServer::~WebServer() {
    for (size_t i = 0; i < listeners.size(); ++i) {
        close(listeners[i].fd);
    }
    for (size_t i = 0; i < listeners.size(); ++i) {
        freeaddrinfo(listeners[i].servinfo);
    }
    listeners.clear();
}
void WebServer::_addEvent(std::vector<struct kevent> &events,
                          uintptr_t ident, int16_t filter, uint16_t flags,
                          uint32_t fflags, intptr_t data, void* udata) {
    struct kevent ev;
    EV_SET(&ev, ident, filter, flags, fflags, data, udata);
    events.push_back(ev);
}

void WebServer::registerEvents() {
    listenerEvents.clear();
    listenerEvents.reserve(listeners.size());

    for (size_t i = 0; i < listeners.size(); i++) {
        int fd = listeners[i].fd;
        _addEvent(listenerEvents, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)1);
    }

    if (kevent(kq, listenerEvents.data(), listenerEvents.size(), NULL, 0, NULL) == -1) {
        throw std::runtime_error("Failed to register listener events");
    }
}

void WebServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("fcntl(F_GETFL) failed");

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl(F_SETFL) failed");
}

void WebServer::_handleAccept(int listen_fd) {
    int client_fd = accept(listen_fd, NULL, NULL);
    if (client_fd == -1)
        throw std::runtime_error("Failed to accept connection");

    int yes = 1;
    setsockopt(client_fd, SOL_SOCKET, SO_NOSIGPIPE, &yes, sizeof(yes));
    setNonBlocking(client_fd);

    struct kevent ev[3];
    EV_SET(&ev[0], client_fd, EVFILT_READ,  EV_ADD | EV_ENABLE, 0, 0, NULL);
    EV_SET(&ev[1], client_fd, EVFILT_WRITE, EV_ADD | EV_DISABLE, 0, 0, NULL);
    EV_SET(&ev[2], client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timeout, NULL);

    if (kevent(kq, ev, 3, NULL, 0, NULL) == -1) {
        close(client_fd);
        throw std::runtime_error("Failed to register client events");
    }
    clientRequests[client_fd] = HttpRequest();
    std::cout << "Client connected: " << client_fd << std::endl;
}

int WebServer::_handleReadable(int client_fd) {
    char buffer[1024];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
    if (n == 0 || n == -1) return DISCONNECTED;
    
    clientRequests[client_fd].RequestData.append(buffer, n);
clientRequests[client_fd].parse_request();
    {

        struct kevent ev[2];
        EV_SET(&ev[0], client_fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
        EV_SET(&ev[1], client_fd, EVFILT_WRITE, EV_ENABLE, 0, 0, NULL);
        kevent(kq, ev, 2, NULL, 0, NULL);
    }

    // Reset the timer
    struct kevent ev;
    EV_SET(&ev, client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timeout, NULL);
    kevent(kq, &ev, 1, NULL, 0, NULL);

    return CONNECTED;
}

int WebServer::_handleWritable(int client_fd) {
    Response response;
    getDataFromRequest(clientRequests[client_fd], response);
    response.execute_method();
    std::string message = response.getResponse();
    ssize_t n = send(client_fd, message.c_str(), message.length(), 0);
    if (n == -1)
        return DISCONNECTED;

    //if data send successfully, we close the connection
    // std::cout << "Response sent to client: " << client_fd << std::endl;
    return DISCONNECTED;
}

void WebServer::_closeConnection(int fd) {
    struct kevent ev[3];
    EV_SET(&ev[0], fd, EVFILT_READ,  EV_DELETE, 0, 0, NULL);
    EV_SET(&ev[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    EV_SET(&ev[2], fd, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
    kevent(kq, ev, 3, NULL, 0, NULL);

    close(fd);
    std::cout << "Connection closed: " << fd << std::endl;
}

void WebServer::startServer() {
    kq = kqueue();
    if (kq == -1) throw std::runtime_error("Failed to create kqueue");

    registerEvents();

    std::vector<struct kevent> evlist;
    evlist.resize(listeners.size());

    while (true) {
        int nev = kevent(kq, NULL, 0, evlist.data(), evlist.size(), NULL);
        if (nev == -1) continue;

        for (int i = 0; i < nev; ++i) {
            struct kevent &e = evlist[i];
            bool disconnect = false;

            if (e.filter == EVFILT_READ)
            {
                if (e.udata == (void*)1)
                {
                    _handleAccept(e.ident);
                    evlist.resize(evlist.size() + 3);
                }
                else if (_handleReadable(e.ident) == DISCONNECTED)
                    disconnect = true;
            } 
            else if (e.filter == EVFILT_WRITE && _handleWritable(e.ident) == DISCONNECTED)
                disconnect = true;
            else if (e.filter == EVFILT_TIMER)
                disconnect = true;
            if (disconnect)
            {
                _closeConnection(e.ident);
                evlist.resize(evlist.size() - 3);
            }
        }
    }
}
