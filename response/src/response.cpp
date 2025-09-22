#include "../includes/response.hpp"

void Response::setMethod(std::string _method) {
    method = _method;
}

void Response::setPath(std::string _path) {
    path = _path;
}

void Response::setHeader(Header copyHeader) {
    responseHeader.status_line.HttpVersion = copyHeader.status_line.HttpVersion;
    responseHeader.status_line.statusCode = copyHeader.status_line.statusCode;
    responseHeader.status_line.reasonPhrase = copyHeader.status_line.reasonPhrase;
    responseHeader.field_line["Server"] = "WebServer/1.1.0";
    // check for the status from request parsing

    if (responseHeader.status_line.statusCode != OK) {
        std::stringstream ss;

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

void Response::execute_method() {
    if (method == "GET") {
        return (Get());
    } else if (method == "DELETE") {
        return (Delete());
    }
}

void Response::Get() {
    if (!pathExists(path)) {
        // respond with 404 code status
    }

    else if (FileR_OK(path)) {
        // respond with 403 code status
    }
    else {
        //respond with 200 code status
    }
}

void Response::Delete() {

}


int main() {
    std::cout << getTimeOftheDay() << std::endl;
}