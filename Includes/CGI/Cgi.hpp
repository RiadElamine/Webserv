#pragma once

#include "../brain.hpp"

class Cgi : virtual public brain {
    private:
        int       cgi_stdout;
        int       cgi_stdin;
        pid_t     cgi_pid;
        bool      is_stdout_done;
        int       status;
        std::string cgi_dir;

    public:
        Cgi();
        ~Cgi();
        
        // high-level CGI execution
        void openCgiOutputFile();
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
        std::string generateRandomFilename();

        // child process helpers
        void executeCgiScript();
        void changeToCgiDirectory();
        void setupCgiPipes();
        bool hasRequestBody();
        void setupCgiStdin();
        void setupCgiStdout();
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