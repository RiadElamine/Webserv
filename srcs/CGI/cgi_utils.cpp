
#include "../../Includes/CGI/Cgi_handler.hpp"

void Cgi::setNonBlockCloexec(int fd) {
    if (fcntl(fd, F_SETFL, FD_CLOEXEC | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl(F_SETFL) failed");
}

void Cgi::makestdoutDone() { 
        is_stdout_done = true;
        removeCgiEventsFromKqueue(cgi_stdout, -1);
}

// Getters
int Cgi::getCgiOutputFd() const {
     return cgi_stdout; 
}

int Cgi::getCgiPid() const { 
    return cgi_pid; 
}

int Cgi::getClientFd() const { 
    return client_fd; 
}

int &Cgi::getStatus() { 
    return status; 
}

bool Cgi::isStdoutDone() const { 
    return is_stdout_done; 
}
