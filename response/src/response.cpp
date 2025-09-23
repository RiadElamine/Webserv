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
    // check for the status from request parsing

    if (responseHeader.status_line.statusCode != OK) {
        std::stringstream ss;
        body = makeBodyResponse(responseHeader.status_line.reasonPhrase, \
                                responseHeader.status_line.statusCode, "");

        ss << body.length();
        fillFieldLine(responseHeader.field_line, "text/html", ss.str());

        //send Response
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
    e_StatusCode statusCode;
    std::string mime;
    std::stringstream ss;

    if (!pathExists(path)) {
        // respond with 404 code status
        statusCode = Not_Found;
        mime = "text/html";
        body = makeBodyResponse(getReasonPhrase(statusCode), statusCode, "");
    } 
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
    responseHeader.status_line.reasonPhrase = getReasonPhrase(OK);
    ss << body.length();
    fillFieldLine(responseHeader.field_line, mime, ss.str());
    //send response
}

void Response::Delete() {

}

int main() {
    std::cout << getTimeOftheDay() << std::endl;
}