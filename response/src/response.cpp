#include "../includes/response.hpp"

std::string getTimeOftheDay() {
    std::time_t t = std::time(0);        // get current time
    std::tm* gmt = std::gmtime(&t);      // convert to UTC/GMT

    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", gmt);
    return std::string(buffer);
}


void Response::setHeader(Header copyHeader) {
    responseHeader.status_line.HttpVersion = copyHeader.status_line.HttpVersion;
    responseHeader.status_line.statusCode = copyHeader.status_line.statusCode;
    responseHeader.status_line.reasonPhrase = copyHeader.status_line.reasonPhrase;
    responseHeader.field_line["Server"] = "WebServer/1.1.0";
    // check for the status from request parsing
  
    if (responseHeader.status_line.statusCode != 200) {
        std::stringstream ss;
        // send_response
        responseHeader.field_line["Date"] = getTimeOftheDay();
        responseHeader.field_line["Content-Type"] = "text/html";
        responseHeader.field_line["Connection"] = "keep-alive";
        body.append("<!DOCTYPE HTML>\n<title>");
        ss << responseHeader.status_line.statusCode;
        body.append(ss.str() + responseHeader.status_line.reasonPhrase + "</title>");
        body.append("<h1>" + responseHeader.status_line.reasonPhrase + "</h1>");
        ss.str("");
        ss.clear();
        ss << body.length();
        responseHeader.field_line["Content-Length"] = ss.str();
    }
}

void Response::execute_method(HttpRequest request_message __attribute__((unused))) {
    // if request
}

int main() {
    std::cout << getTimeOftheDay() << std::endl;
}