
#include "../Includes/Cgi_handler.hpp"


Cgi::Cgi(KqueueContext &Context) : Context(Context)
{
    this->client_fd = Context.event.ident;
    is_stdout_done = false;
    status = 0;
    cgi_pid = -1;

    // i need to gernerate random name for the cgi script output file
  
};

Cgi::~Cgi()
{
    // cleanup
    if (cgi_stdout != -1)
        close(cgi_stdout);
    if (cgi_stdin != -1)
        close(cgi_stdin);
    if (cgi_pid != -1)
    {
        kill(cgi_pid, SIGKILL);
        // reap CGI process to avoid zombie
        int status;
        if (waitpid(cgi_pid, &status, WNOHANG) == -1){ 
            perror("waitpid"); 
        }
    }
}


void Cgi::executeCgi()
{
    // Open file to write CGI script output
    cgi_stdout = open("cgi_output.txt", O_CREAT | O_RDWR, 0644);
    if (cgi_stdout == -1)
        throw std::runtime_error("Failed to open cgi_output.txt");

    // set both ends of my tunnel to non-blocking and close on exec
    setNonBlockCloexec(cgi_stdout);
    
    redirectCgiOutput();

    // Fork a new process to execute the CGI script
    cgi_pid = fork();
    if (cgi_pid == -1) {
        perror("fork");
        close(cgi_stdin);
        close(cgi_stdout);
        throw std::runtime_error("Fork failed");
    }
    else if (cgi_pid == 0) {
        executeCgiScript();
    } 
    else
    {
        setupParentProcessEvents();
    }
}

void Cgi::_readCgiOutput() {
    char buffer[4096];
    std::cout << cgi_stdout << std::endl;
    ssize_t n = read(cgi_stdout, buffer, sizeof(buffer));
    if (n <= 0)
    {
        perror("read");
        // remember to remove the file
        Context.state_of_connection = DISCONNECTED;
        return;
    }
      
     if (parseCGIheader(buffer, n, Context.clientResponses[client_fd]))
     {
         std::cout << "--CGI output read done for client: " << client_fd << std::endl;
         makestdoutDone();
         return;
     }

     // Reset the timer
     struct kevent ev;
     EV_SET(&ev, cgi_stdout, EVFILT_TIMER, EV_ENABLE , 0, timeout, (void*)this);
     if (kevent(Context.kq, &ev, 1, NULL, 0, NULL) == -1) {
         perror("kevent");
         throw std::runtime_error("Failed to reset timer");
     }
    
     std::cout << "--CGI output read completed for client: " << client_fd << std::endl;
}

void Cgi::finalizeCgiProcess(int statusCode) {

    // set error status (500 or 504, etc.)
    if (statusCode != I_Dont_have_respons)
    {
        if (statusCode == Doesnt_fail)
        {
            // send error response to client
            Context.clientResponses[this->getClientFd()].setStatusCode(statusCode);
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

// Handle CGI process completion
void Cgi::handleCgiCompletion()
{
    // reap the cgi process to avoid zombie
    if (waitpid(this->getCgiPid(), &this->getStatus(), 0) == -1)
    {
        perror("waitpid");
        throw std::runtime_error("Failed to read CGI process");
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
        std::cout << "--CGI process completed successfully for client: " << client_fd << std::endl;
        return;
    }
    else
    {
        // CGI exited with error
        // send 502 Bad Gateway response to client
        _status_code = Bad_Gateway;
        std::cout << "--CGI process failed for client: " << client_fd << std::endl;
    }
    this->finalizeCgiProcess(_status_code);
}
