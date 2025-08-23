#include "../Includes/Webserver.hpp"

WebServer::WebServer(std::vector<ServerConfig>  &servers) {
    for (size_t i = 0; i < servers.size(); ++i) {
        for (std::set<std::pair<std::string, uint16_t> >::iterator it = servers[i].listens.begin(); it != servers[i].listens.end(); ++it) {
            Listener listener;

            listener.hosts = &servers[i]; 

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
            if (setsockopt(listener.fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
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

void WebServer::_addEvent(uintptr_t ident, int16_t filter, uint16_t flags,
                           uint32_t fflags, intptr_t data, void* udata) {
    struct kevent ev;
    EV_SET(&ev, ident, filter, flags, fflags, data, udata);
    events.push_back(ev);
}

void WebServer::registerEvents() {
    events.reserve(listeners.size() * 3);
    for (size_t i = 0; i < listeners.size(); i++) {
        int fd = listeners[i].fd;
        _addEvent(fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)1);
    }

    if (kevent(kq, events.data(), events.size(), NULL, 0, NULL) == -1) {
        throw std::runtime_error("Failed to register events");
    }
}

void WebServer::_handleAccept(int listen_fd) {

    int client_fd = accept(listen_fd, NULL, NULL);
    if (client_fd == -1) {
        throw std::runtime_error("Failed to accept connection");
    }

    _addEvent(client_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    _addEvent(client_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
    _addEvent(client_fd, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timeout, NULL);

   if (kevent(kq, &events[events.size() - 3], 3, NULL, 0, NULL) == -1) {
        close(client_fd);
        throw std::runtime_error("Failed to register new client connection");
    }

}

void WebServer::_handleReadable(int client_fd) {
    char buffer[1024];
    ssize_t bytesRead = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytesRead == 0) {
        perror("recv");
        _closeConnection(client_fd);
        return;
    }
    write(1, buffer, bytesRead);
    buffer[bytesRead] = '\0';
}

void WebServer::_handleWritable(int client_fd) {
    const char* msg = "HTTP/1.1 200 OK\r\nContent-Length: 5\r\n\r\nHello";
    send(client_fd, msg, strlen(msg), 0);


    _closeConnection(client_fd);
}

void WebServer::_closeConnection(int fd) {
    if (fd < 0) {
        std::cout << "waiting for close connection: " << fd << std::endl;
        return;
    }
    close(fd);
    struct kevent tmp[3];
    EV_SET(&tmp[0], fd, EVFILT_READ,  EV_DELETE, 0, 0, NULL);
    EV_SET(&tmp[1], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    EV_SET(&tmp[2], fd, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);

    if (kevent(kq, &tmp[0], 3, NULL, 0, NULL) == -1) {
        perror("kevent EV_DELETE");
    }

    for (size_t i = 0; i < events.size();) {
        if ((int)events[i].ident == fd) {
            events.erase(events.begin() + i);
            std::cout << "Connection closed: " << fd << std::endl;
        }
        else
        {
            i++;
        }
    }
}

void WebServer::sartServer() {
    kq = kqueue();
    if (kq == -1) {
        throw std::runtime_error("Failed to create kqueue");
    }

    registerEvents();

    std::vector<struct kevent> evlist;
    evlist.resize(events.size());
    while (true) {
        int nev = kevent(kq, NULL, 0, evlist.data(), evlist.size(), NULL);
        if (nev == -1) {
            throw std::runtime_error("kevent error");
        }
        for (int i = 0; i < nev; ++i) {
            switch (evlist[i].filter)
            {
                case EVFILT_READ:
                {
                    if (evlist[i].udata == (void*)1)
                    {
                        _handleAccept(evlist[i].ident);
                        break;
                    }
                    else
                    {
                        _handleReadable(evlist[i].ident);
                    }
                }
                case EVFILT_WRITE:
                {
                    _handleWritable(evlist[i].ident);
                    break;
                }
                case EVFILT_TIMER:
                {
                    _closeConnection(evlist[i].ident);
                    i++;
                    break;
                }
            }
        }
    }
}
