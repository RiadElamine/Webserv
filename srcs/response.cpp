#include "../Includes/response.hpp"


Response::Response() {
    transferEncoding = false;
}


void Response::setMethod(std::string _method) {
    method = _method;
}

void Response::setServer(ServerConfig *server) {
    currentServer = server;
}

void Response::setHeader(Header copyHeader) {
    responseHeader.status_line.HttpVersion = copyHeader.status_line.HttpVersion;
    responseHeader.status_line.statusCode = copyHeader.status_line.statusCode;
    responseHeader.status_line.reasonPhrase = copyHeader.status_line.reasonPhrase;
    // check for the status from request parsing

    if (responseHeader.status_line.statusCode != OK) {
        std::stringstream ss;
        body = makeBodyResponse(responseHeader.status_line.reasonPhrase, \
                                responseHeader.status_line.statusCode, "");

        ss << body.length();
        fillFieldLine(responseHeader.field_line, "text/html", ss.str());
    }
}

void Response::execute_method() {
    if (method == "GET") {
        return (this->Get());
    } else if (method == "DELETE") {
        return (this->Delete());
    }
}

//modify path

void Response::Get() {
    e_StatusCode statusCode;
    std::string mime;
    std::stringstream ss;
    Location *currentLocation = getCurrentLocation(path, currentServer);
    path = currentLocation->root + path;
    std::cout << "root: " << currentLocation->root  << std::endl;

    if (!pathExists(path)) {
        // respond with 404 code status
        statusCode = Not_Found;
        mime = "text/html";
        body = makeBodyResponse(getReasonPhrase(statusCode), statusCode, "");
    }
    //need to check if the file has GET method
    else if (!FileR_OK(path)) {
        // respond with 403 code status
        statusCode = Forbidden;
        mime = "text/html";
        body = makeBodyResponse(getReasonPhrase(statusCode), statusCode, "");
    }
    else {
        //respond with 200 code status
        statusCode = OK;
        mime = getMIME(path);
        body = makeBodyResponse(getReasonPhrase(statusCode), statusCode, path);
    }

    responseHeader.status_line.statusCode = statusCode;
    responseHeader.status_line.reasonPhrase = getReasonPhrase(statusCode);
    ss << body.length();
    ss.str();
    ss.clear();
    fillFieldLine(responseHeader.field_line, mime, ss.str());
}

void Response::Delete() {

}

std::string Response::getResponse() {
    std::string message;
    std::stringstream ss;

    ss << responseHeader.status_line.statusCode ;
    message += responseHeader.status_line.HttpVersion + " " + ss.str() + " " + responseHeader.status_line.reasonPhrase + "\n";
    message += "Date: " + responseHeader.field_line["Date"] + "\r\n";
    message += "Content-Type: " + responseHeader.field_line["Content-Type"] + "\r\n";
    message += "Content-Length: " + responseHeader.field_line["Content-Length"] + "\r\n";
    message += "Connection: " + responseHeader.field_line["Connection"] + "\n";
    message += "Server: " + responseHeader.field_line["Server"] + "\r\n";
    message += "\r\n" + body;
    ss.str("");
    ss.clear();
    return message;
}