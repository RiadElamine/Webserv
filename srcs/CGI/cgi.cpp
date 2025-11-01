#include "../../Includes/CGI/Cgi.hpp"
#include "../../Includes/utils.hpp"

Cgi::Cgi()
{
    is_stdout_done = false;
    status = 0;
    cgi_pid = -1;
    cgi_stdin = -1;
    cgi_stdout = -1;
};

Cgi::~Cgi()
{
    // cleanup
    if (cgi_stdout != -1) {
        close(cgi_stdout);
        cgi_stdout = -1;
    }
    if (cgi_stdin != -1) {
        close(cgi_stdin);
        cgi_stdin = -1;
    }
    if (cgi_pid != -1)
    {
        kill(cgi_pid, SIGKILL);
        // reap CGI process to avoid zombie
        int status;
        waitpid(cgi_pid, &status, WNOHANG);
    }
    
    // Only attempt to remove the file if it exists and filename is not empty
    if (!filename_cgi_output.empty())
    {
        std::cout << "Removing temporary CGI output file: " << filename_cgi_output << std::endl;
        if (std::remove(filename_cgi_output.c_str()) != 0)
        {
            std::cerr << "Warning: Failed to remove temporary CGI output file: " << filename_cgi_output << " - " << strerror(errno) << std::endl;
        }
    }
}

void Cgi::openCgiOutputFile() {
    cgi_dir = currentLocation->root + currentLocation->Route + "/";

    filename_cgi_output = generateRandomFilename();

    std::string path_cgi_output_file = cgi_dir + filename_cgi_output;

    cgi_stdout = open(path_cgi_output_file.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
    if (cgi_stdout == -1)
        throw std::runtime_error("Failed to open cgi_output file");
}

void Cgi::executeCgi()
{

    openCgiOutputFile();
    
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
        filename_cgi_output = cgi_dir +  filename_cgi_output;
        setNonBlocking(cgi_stdout);
        setupParentProcessEvents();
    }
}

void Cgi::_readCgiOutput() {
    char buffer[4096];
    ssize_t n = read(cgi_stdout, buffer, sizeof(buffer));
    if (n <= 0)
    {
        state_of_connection = DISCONNECTED;
        return;
    }

    if (parseCGIheader( headers_buffer_CGI, buffer, n, *(Context->clientResponses[this->getClientFd()])))
     {
        makestdoutDone();
        if (!Context->clientResponses[client_fd]->open_stream(filename_cgi_output))
            throw std::runtime_error("Can't open the cgi output file");
        return;
     }

     // Reset the timer
     struct kevent ev;
     EV_SET(&ev, cgi_stdout, EVFILT_TIMER, EV_ENABLE , 0, timeout, (void*)this);
     if (kevent(Context->kq, &ev, 1, NULL, 0, NULL) == -1) {
         perror("kevent");
         throw std::runtime_error("Failed to reset timer");
     }
}

void Cgi::finalizeCgiProcess(int statusCode) {

    // set error status (500 or 504, etc.)
    if (statusCode != I_Dont_have_respons)
    {
        if (statusCode != Doesnt_fail)
        {
            // send error response to client
            Context->clientResponses[this->getClientFd()]->setStatusCode(statusCode);
        }
        // enable write event on client socket to send response
        // disable read event on client socket
        // reset timer
        std::vector<struct kevent> ev;
        _addEvent(ev, this->getClientFd(), EVFILT_READ,  EV_DISABLE, 0, 0, (void *)client_event);
        _addEvent(ev, this->getClientFd(), EVFILT_WRITE, EV_ENABLE, 0, 0, (void *)client_event);
        if (kevent(Context->kq, ev.data(), ev.size(), NULL, 0, NULL) == -1) {
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
    waitpid(this->getCgiPid(), &this->getStatus(), 0);
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
