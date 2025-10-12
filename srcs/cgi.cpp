#include "../Includes/response.hpp"
#define BUFFER_SIZE 10



// Trim leading and trailing spaces
static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

void parseCGIheader(std::ifstream& buffer) {
    // std::istringstream iss(cgiOutput);
    Response response;
    Header responseHeader;
    std::string line;
    bool headerDone = false;
    std::string body;

    while (std::getline(buffer, line) && line != "\r\n") {
        // Remove possible '\r' at line end
        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);

        if (!headerDone) {
            if (line.empty()) {
                // Empty line = end of headers
                headerDone = true;
                continue;
            }

            // Full HTTP response line
            // if (line.find("HTTP/") == 0) {
            //     res.headers["__FULL_HTTP__"] = line; // mark full HTTP
            //     continue;
            // }

            // Split key:value
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));
                if (key == "Status")
                {
                    std::stringstream ss(value);
                    int val ;
                    ss >> val;
                    responseHeader.status_line.statusCode = (e_StatusCode) val;
                    responseHeader.status_line.HttpVersion = "HTTP/1.1";
                    responseHeader.status_line.reasonPhrase = getReasonPhrase((e_StatusCode) val);
                } else
                    responseHeader.field_line[key] = value;
            }
        }
    }
    response.setHeader(responseHeader);
    response.setField_line(responseHeader.field_line);
    std::cout << response.getResponse();   
}


void makeCGIbody(std::ifstream& buffer __attribute__ ((unused))) {
    
}

std::string readCGI(std::string filename) {
	// Response response;
	std::ifstream buffer(filename, std::ifstream::in);

	parseCGIheader(buffer);
	makeCGIbody(buffer);

	// return response.getResponse()
    return "";

}


void executeCGI(std::string outFile, char* args[]) {
    int fd = open(outFile.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
        throw std::runtime_error("Error while opening the file!!!");
    int saved_stdout = dup(1);        // Save original stdout
    dup2(fd, 1);                      // Redirect stdout to file
    close(fd);                        // fd no longer needed

    pid_t pid = fork();
    if (pid == 0) {
        execve(args[0], args, NULL);
        exit(1);
    } else if (pid == -1) {
        throw std::runtime_error("Can't create child process");
    }
    waitpid(pid, NULL, 0);

    // Restore stdout
    dup2(saved_stdout, 1);
    close(saved_stdout);
}

//int main() {
//     char *args[] = {
//            (char*) "/usr/local/bin/python3",
//            (char *) "/Users/oel-asri/Kingsave/Webserv/cgi-bin/cgi/index.py",
//            NULL
//        };
//        executeCGI("/Users/oel-asri/Kingsave/Webserv/CGI", args);
//        readCGI("/Users/oel-asri/Kingsave/Webserv/CGI");
//    return (0);
//}


