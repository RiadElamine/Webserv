#include "../includes/response.hpp"

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
    if (statusCode != 200) {
        std::stringstream ss;

        body.append("<!DOCTYPE HTML>\n<title>");
        ss << statusCode;
        body.append(ss.str() + reasonPhrase + "</title>");
        body.append("<h1>" + ss.str() + reasonPhrase + "</h1>");
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

}

