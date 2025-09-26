#include "../Includes/response.hpp"

std::string getTimeOftheDay() {
    std::time_t t = std::time(0);        // get current time
    std::tm* gmt = std::gmtime(&t);      // convert to UTC/GMT

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return std::string(buffer);
}

std::string getReasonPhrase(e_StatusCode statusCode) {
    switch (statusCode) {
        case Continue: return "Continue";
        case OK: return "OK";
        case Created: return "Created";
        case No_Content: return "No Content";
        case Bad_Request: return "Bad Request";
        case Unauthorized: return "Unauthorized";
        case Forbidden: return "Forbidden";
        case Not_Found: return "Not Found";
        case Method_Not_Allowed: return "Method Not Allowed";
        case Request_Timeout: return "Request Timeout";
        case Conflict: return "Conflict";
        case Payload_Too_Large: return "Payload Too Large";
        case Internal_Server_Error: return "Internal Server Error";
        case Not_Implemented: return "Not Implemented";
        case Bad_Gateway: return "Bad Gateway";
        case Service_Unavailable: return "Service Unavailable";
        default:
            throw std::runtime_error("Status Code Not Implemented");
    }
}

bool pathExists(std::string path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool FileR_OK(std::string path) {
    return (access(path.c_str(), R_OK) == 0);
}

std::string makeBodyResponse(std::string reasonPhrase, int statusCode, std::string path) {
    std::string body;
    if (path.empty()) {
        std::stringstream ss;

        body.append("<!DOCTYPE HTML>\n<title>");
        ss << statusCode;
        body.append(ss.str() + " " + reasonPhrase + "</title>\n");
        body.append("<h1>" + ss.str() + " " + reasonPhrase + "</h1>\n");
    } else {
        if (!isCGI(path)) {
            body = readFile(path);
        } else {
            body = getCGI(path);
        }
    }

    return body;
}

void fillFieldLine(std::map<std::string, std::string> &field_line, std::string contentType, std::string contentLength) {
    field_line["Date"] = getTimeOftheDay();
    field_line["Content-Type"] = contentType;
    field_line["Content-Length"] = contentLength;
    field_line["Connection"] = "keep-alive";
    field_line["Server"] = "WebServer/1.1.0";
}

std::string getMIME(std::string path) {
    std::string mime;
    size_t index = path.rfind(".");

    if (index == std::string::npos || index == path.length() - 1) return "application/octet-stream";
    mime = path.substr(index + 1);

    for (size_t i = 0; i < mime.length(); i++) {
        mime[i] = static_cast<char>(std::tolower(static_cast<unsigned char>(mime[i])));
    }

    if (mime == "html" || mime == "htm") return "text/html";
    if (mime == "css") return "text/css";
    if (mime == "js") return "application/javascript";
    if (mime == "png") return "image/png";
    if (mime == "jpg" || mime == "jpeg") return "image/jpeg";
    if (mime == "gif") return "image/gif";
    if (mime == "txt") return "text/plain";
    return "application/octet-stream";
}

std::string readFile(std::string path) {
    // need optimization
    std::ifstream file;

    file.open(path.c_str(), std::ifstream::binary);

    if (file) {
        file.seekg(0, file.end);
        size_t length = file.tellg();
        file.seekg(0, file.beg);

        char *buffer = new char[length];
        file.read(buffer, length);
        file.close();
        std::string content(buffer, length);
        delete[] buffer;
        return content;
    } 
    throw std::runtime_error("Error while opening the file");
}

std::string getCGI(std::string path __attribute__ ((unused))) {
    return "";
}

bool isCGI(std::string) {
    return false;
}

bool isPrefix(const std::string& prefix, const std::string& str) {
    if (prefix.size() > str.size())
        return false;
    return str.compare(0, prefix.size(), prefix) == 0;
}

Location* getCurrentLocation(std::string oldPath, ServerConfig *currentServer) {
    Location *currentLocation = nullptr;
    Location *fallBack = nullptr;
    size_t longestMatch = 0;

    for (size_t i = 0; i < currentServer->locations.size(); i++) {
        const Location &loc = currentServer->locations[i];

        if (loc.URI == "/") {
            fallBack = &currentServer->locations[i];
        }

        if (isPrefix(loc.URI, oldPath)) {
            if (loc.URI.size() > longestMatch) {
                longestMatch = loc.URI.size();
                currentLocation = &currentServer->locations[i];
            }
        }
    }

    if (!currentLocation)
        currentLocation = fallBack;

    if (currentLocation->root.empty())
        currentLocation->root = currentServer->global_root;
    if (currentLocation->index.empty())
        currentLocation->index = currentServer->global_index;

    return currentLocation;
}