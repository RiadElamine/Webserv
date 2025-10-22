#pragma once

// C++ Standard Library
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <utility>
#include <algorithm>
#include <iterator>
#include <limits>
#include <cctype>
#include <cstring>
#include <ctime>
#include <stdexcept>

// POSIX / System Headers
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/event.h>
#include <fcntl.h>
#include <netdb.h>
#include <dirent.h>
#include <signal.h>

// Project-specific headers
#include "status_code.hpp"

// Connection state
enum ConnectionState {
    CONNECTED,
    DISCONNECTED
};

// Timeout duration
#define timeout 10 // 


// type of event
enum type_event {
    server_event,
    client_event,
};


// Helper function to add events to a vector
void _addEvent(std::vector<struct kevent> &events,
                          uintptr_t ident, int16_t filter, uint16_t flags,
                          uint32_t fflags, intptr_t data, void* udata);

#define  DEFAULT_PAGE_ERRORS "./default_error_page/"
#define DEFAULT_ROOT "./www"
