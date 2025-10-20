
#include "../Includes/Cgi_handler.hpp"

// Setup kqueue events in the parent process after forking CGI
void Cgi::setupParentProcessEvents()
{
    std::cout << "--CGI process started with PID: " << cgi_pid << " for client: " << client_fd << std::endl;
    std::vector<struct kevent> ev;
    addCgiReadAndTimeoutEvents(ev);
    disableClientEvents(ev);
    registerKqueueEvents(ev);
    monitorCgiProcessExit();
}

void Cgi::addCgiReadAndTimeoutEvents(std::vector<struct kevent> &ev)
{
    // Monitor reading from CGI stdout
    _addEvent(ev, cgi_stdout, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, (void*)this);

    // Monitor CGI timeout
    _addEvent(ev, cgi_stdout, EVFILT_TIMER, EV_ADD | EV_ENABLE, 0, timeout, (void*)this);
}

void Cgi::disableClientEvents(std::vector<struct kevent> &ev)
{
    // Disable read event on client socket
    _addEvent(ev, client_fd, EVFILT_READ, EV_DISABLE, 0, 0, (void*)client_event);

    // Disable timeout event on client socket
    _addEvent(ev, client_fd, EVFILT_TIMER, EV_DISABLE, 0, 0, (void*)client_event);
}

void Cgi::registerKqueueEvents(std::vector<struct kevent> &ev)
{
    if (kevent(Context.kq, ev.data(), ev.size(), NULL, 0, NULL) == -1)
        throw std::runtime_error("Failed to register CGI events");
}

void Cgi::monitorCgiProcessExit()
{
    struct kevent ev_proc;
    EV_SET(&ev_proc, cgi_pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0, (void*)this);
    if (kevent(Context.kq, &ev_proc, 1, NULL, 0, NULL) == -1)
        throw std::runtime_error("Failed to monitor CGI process");
}


// Remove CGI-related events from kqueue
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
        std::cout << "--CGI events removed from kqueue for client: " << client_fd << std::endl;
        cgi_stdout = -1;
    }
}
