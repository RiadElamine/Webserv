
#include "../../Includes/CGI/Cgi.hpp"

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


std::string Cgi::generateRandomFilename() {
    const char chars[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    int num_chars = 62;

    std::time_t t = std::time(0);
    std::srand(t);

    std::string random_str;
    for (int i = 0; i < 10; ++i)
    {
        random_str += chars[std::rand() % num_chars];
    }

    std::stringstream ss;
    ss  << t  << random_str << std::hex << &num_chars;
    return ss.str();
}