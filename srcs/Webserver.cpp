#include "../Includes/Webserver.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

    for (std::map<int, ServerConfig*>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
        int fd = it->first;
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
    clientRequests[client_fd].setServer(listeners[listen_fd]);
    std::cout << "Client connected: " << client_fd << std::endl;
}

int WebServer::_handleReadable(int client_fd) {
    char buffer[1024];
    ssize_t n = recv(client_fd, buffer, sizeof(buffer), 0);
    if (n == 0) return DISCONNECTED;
    if (n == -1) return CONNECTED;
    
    if (clientRequests[client_fd].parse_request(buffer, n))
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
