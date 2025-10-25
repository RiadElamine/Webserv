#pragma once
#include "../Common.hpp"
#include "../Request/HttpRequest.hpp"

class HttpRequest;
class Cgi;

// Connection state
enum ConnectionState {
    CONNECTED,
    DISCONNECTED
};

struct KqueueContext {
    std::vector<struct kevent>  evlist;
    struct kevent               event;
    int                         kq;
    std::map<int, HttpRequest>  clientRequests;
    std::map<int, Cgi>          clientCgiProcesses;
    std::map<int, Response>     clientResponses;
    std::map<int, ConnectionState> state_of_connection;
    std::map<int, std::string>  headers_buffer_CGI;
};

class Cgi {
    private:
        int       cgi_stdout;
        int       cgi_stdin;
        pid_t     cgi_pid;
        bool      is_stdout_done;
        int       status;
        int       client_fd;
        KqueueContext &Context;

    public:
        Cgi(KqueueContext &Context);
        ~Cgi();
        
        // high-level CGI execution
        void executeCgi();
        void _readCgiOutput();
        void handleCgiCompletion();
        void finalizeCgiProcess(int statusCode);

        // event management
        void removeCgiEventsFromKqueue(int FD, int PROCESS_ID);

        // utils
        int getCgiOutputFd() const; 
        int getCgiPid() const;
        int getClientFd() const;
        int &getStatus();
        bool isStdoutDone() const;
        void setNonBlockCloexec(int fd);
        void makestdoutDone();

        // child process helpers
        void executeCgiScript();
        void setupCgiPipes();
        bool hasRequestBody();
        void setupCgiStdin();
        void setupCgiStdout();
        void setupCgiOuput_Parent();
        void redirectCgiInput();
        void redirectCgiOutput();
        std::vector<char*> buildCgiArgs(const std::string &scriptPath);
        std::vector<char*> buildCgiEnv();
        void runExecve(const char *interpreter, const std::vector<char*> &args, std::vector<char*> &env);

        // parent process helpers
        void setupParentProcessEvents();
        void addCgiReadAndTimeoutEvents(std::vector<struct kevent> &ev);
        void disableClientEvents(std::vector<struct kevent> &ev);
        void registerKqueueEvents(std::vector<struct kevent> &ev);
        void monitorCgiProcessExit();
};