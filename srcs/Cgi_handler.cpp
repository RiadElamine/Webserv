
#include "../Includes/Cgi_handler.hpp"

void Cgi::setNonBlockCloexec(int fd) {
    if (fcntl(fd, F_SETFL, FD_CLOEXEC | O_NONBLOCK) == -1)
        throw std::runtime_error("fcntl(F_SETFL) failed");
}

Cgi::Cgi(KqueueContext &Context, int client_fd) : Context(Context)
{
    this->client_fd = Context.event.ident;
    is_stdout_done = false;
    status = 0;
    cgi_pid = -1;

    // i need to gernerate random name for the cgi script output file
    cgi_stdout = open("cgi_output.txt", O_CREAT | O_RDWR, 0644);
    if (cgi_stdout == -1)
        throw std::runtime_error("Failed to open cgi_output.txt");
    // 
    if (!Context.clientRequests[client_fd].get_filename().empty())
    {
        cgi_stdin = open(Context.clientRequests[client_fd].get_filename().c_str(), O_RDWR);
        if (cgi_stdin == -1)
        {
            close(cgi_stdout);
            throw std::runtime_error("Failed to open body.txt");
        }
        // set both ends of my tunnel to non-blocking and close on exec
        setNonBlockCloexec(cgi_stdin);
        setNonBlockCloexec(cgi_stdout);
    }
    else
        cgi_stdin = -1;
};

void Cgi::executeCgi()
{
    // Fork a new process to execute the CGI script
    cgi_pid = fork();
    if (cgi_pid == -1) {
        perror("fork");
        close(cgi_stdin);
        close(cgi_stdout);
        throw std::runtime_error("Fork failed");
    }
    else if (cgi_pid == 0) {

        // Redirect stdout to the write end of the pipe
        if (dup2(cgi_stdout, STDOUT_FILENO) == -1) {
            close(cgi_stdout);
            throw std::runtime_error("dup2 stdout failed");
        }
        // send body to cgi script via stdin
        if (dup2(cgi_stdin, STDIN_FILENO) == -1) {
            close(cgi_stdout);
            close(cgi_stdin);
            throw std::runtime_error("dup2 stdin failed");
        }
        // Send output to the pipe 
        // this the cgi script execution in execve system call
        // if (execve(CGI_PATH, Args, (char *const *)NULL) == -1)  
        // {
        //     perror("execve");
        //     _exit(1);
        // }
        exit(0);
    } 
    else
    {
        // Parent process

        // Close unused read end of the pipe
        close(cgi_stdin);
    
        std::vector<struct kevent> ev;
        // Add events to kqueue to monitor reading from body of CGI script
        _addEvent(ev, cgi_stdout, EVFILT_READ,  EV_ADD | EV_ENABLE, 0, 0, (void*)this);
        // Add timeout event for CGI process
        _addEvent(ev, cgi_stdout, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timeout, (void*)this);
        // disable read event on client socket while CGI is running
        _addEvent(ev, this->client_fd, EVFILT_READ, EV_DISABLE, 0, 0, (void*)client_event);
        // disable timeout event on client socket while CGI is running
        _addEvent(ev, this->client_fd, EVFILT_TIMER, EV_DISABLE, 0, 0, (void*)client_event);
        // Register the events
        if (kevent(Context.kq, ev.data(), ev.size(), NULL, 0, NULL) == -1)
            throw std::runtime_error("Failed to register CGI events");

        // Monitor the CGI process for exit
        struct kevent ev_proc;
        EV_SET(&ev_proc, cgi_pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, (void*)this);
        if (kevent(Context.kq, &ev_proc, 1, NULL, 0, NULL) == -1)
            throw std::runtime_error("Failed to monitor CGI process");
    
    }
}

void  Cgi::makestdoutDone(){ 
        is_stdout_done = true;
        removeCgiEventsFromKqueue(cgi_stdout, -1);
}

void Cgi::_readCgiOutput() {
    char buffer[4096];
    ssize_t n = read(cgi_stdout, buffer, sizeof(buffer));
    if (n <= 0)
    {
        perror("read");
        // remember to remove the file
        Context.state_of_connection = DISCONNECTED;
        return;
    }

    // requestCgi.RequestData.append(buffer, n);
    // if (parseCGIheader(buffer, response))
    // {
    //     makestdoutDone();
    //     Context.state_of_connection = CONNECTED;
    //     return;
    // }

    // Reset the timer
    struct kevent ev;
    EV_SET(&ev, cgi_stdout, EVFILT_TIMER, EV_ENABLE, 0, timeout, (void*)this);
    if (kevent(Context.kq, &ev, 1, NULL, 0, NULL) == -1) {
        perror("kevent");
        throw std::runtime_error("Failed to reset timer");
    }

}

void Cgi::removeCgiEventsFromKqueue(int FD, int PROCESS_ID) {

    if (PROCESS_ID >= 0) {
        struct kevent ev;
        EV_SET(&ev, PROCESS_ID, EVFILT_PROC, EV_DELETE, 0, 0, NULL);
        if (kevent(Context.kq, &ev, 1, NULL, 0, NULL) == -1) {
            throw std::runtime_error("Failed to remove CGI process event");
        }
        cgi_pid = -1;
    }

    if (FD >= 0) {
        std::vector<struct kevent> ev_cgiFd;
        _addEvent(ev_cgiFd, FD, EVFILT_READ,  EV_DELETE, 0, 0, NULL);
        _addEvent(ev_cgiFd, FD, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);
        if (kevent(Context.kq, ev_cgiFd.data(), ev_cgiFd.size(), NULL, 0, NULL) == -1) {
            throw std::runtime_error("Failed to remove CGI fd events");
        }
        cgi_stdout = -1;
    }
}

void Cgi::handleCgiFailure(int statusCode) {

    // set error status (500 or 504, etc.)
    if (statusCode != I_Dont_have_respons)
    {
        if (statusCode == Doesnt_fail)
        {
            // send error response to client
            Context.clientRequests[this->getClientFd()].setStatusCode(statusCode);
        }
        // enable write event on client socket to send response
        // disable read event on client socket
        // reset timer
        std::vector<struct kevent> ev;
        _addEvent(ev, this->getClientFd(), EVFILT_READ,  EV_DISABLE, 0, 0, (void *)client_event);
        _addEvent(ev, this->getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, (void *)client_event);
        _addEvent(ev, this->getClientFd(), EVFILT_TIMER, EV_ENABLE, 0, timeout, (void *)client_event);
        if (kevent(Context.kq, ev.data(), ev.size(), NULL, 0, NULL) == -1) {
            throw std::runtime_error("Failed to modify client events");
        }
    }
    // remove cgi events from kqueue
    removeCgiEventsFromKqueue(this->getCgiOutputFd(), this->getCgiPid());
}


void Cgi::handleCgiCompletion()
{
    // reap the cgi process to avoid zombie
    if (waitpid(this->getCgiPid(), &this->getStatus(), 0) == -1)
    {
        perror("waitpid");
        throw std::runtime_error("Failed to reap CGI process");
    }
    int _status_code;

    // CGI finished successfully, dont close client connection yet (we need to send response)
    // prepare to send response to client
    // and remove cgi process events from kqueue

    // check if cgi stdout is Done and exited normally
    if (this->isStdoutDone() && WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        // we dont change the status code, let the response handler do it
        _status_code = Doesnt_fail;
        return;
    }
    else
    {
        // CGI exited with error
        // send 502 Bad Gateway response to client
        _status_code = Bad_Gateway;
    }
    this->handleCgiFailure(_status_code);
}