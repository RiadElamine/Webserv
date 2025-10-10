#pragma once

#include "Common.hpp"
#include "HttpRequest.hpp"

class HttpRequest;
class Cgi;

struct KqueueContext {
    std::vector<struct kevent>  evlist;
    struct kevent               event;
    int                         kq;
    std::map<int, HttpRequest>  clientRequests;
    std::map<int, Cgi>          clientCgiProcesses;
    ConnectionState             state_of_connection;
};

class Cgi {

    private:
        int       cgi_stdout; // this the output of cgi script "headers + body" stored in file (buffer)
        int       cgi_stdin; // this the body of request sent to cgi script
        pid_t     cgi_pid;
        bool      is_stdout_done;
        int       status;
        int       client_fd;

        // get from outside
        KqueueContext &Context;

    public:
        Cgi(KqueueContext &Context, int client_fd);
        void executeCgi();
        void _readCgiOutput();
        void removeCgiEventsFromKqueue(int FD, int PROCESS_ID);
        void handleCgiFailure(int statusCode, bool killAndReap, ConnectionState closeClient);

        // getters
        int getCgiOutputFd() const { return cgi_stdout; }
        int getCgiPid() const { return cgi_pid; }
        int getClientFd() const { return client_fd; }
        int &getStatus() { return status; }
        bool isStdoutDone() const { return is_stdout_done; }

        void setNonBlockCloexec(int fd);

        void makestdoutDone() { is_stdout_done = true; }
};

