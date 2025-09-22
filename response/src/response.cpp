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
                                responseHeader.status_line.statusCode, NULL);

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

    if (!pathExists(path)) {
        // respond with 404 code status
        std::stringstream ss;

        responseHeader.status_line.statusCode = Not_Found;
        responseHeader.status_line.reasonPhrase = getReasonPhrase(Not_Found);
        body = makeBodyResponse(responseHeader.status_line.reasonPhrase, Not_Found, NULL);
        ss << body.length();
        fillFieldLine(responseHeader.field_line, "text/html", ss.str());
    }

    else if (!FileR_OK(path)) {
        // respond with 403 code status
        std::stringstream ss;

        responseHeader.status_line.statusCode = Forbidden;
        responseHeader.status_line.reasonPhrase = getReasonPhrase(Forbidden);
        body = makeBodyResponse(responseHeader.status_line.reasonPhrase, Forbidden, NULL);
        ss << body.length();
        fillFieldLine(responseHeader.field_line, "text/html", ss.str());
    }
    else {
        //respond with 200 code status
        std::stringstream ss;

        responseHeader.status_line.statusCode = OK;
        responseHeader.status_line.reasonPhrase = getReasonPhrase(OK);
        body = makeBodyResponse(responseHeader.status_line.reasonPhrase, OK, path);
        
    }
    //send response
}

void Response::Delete() {

}

int main() {
    std::cout << getTimeOftheDay() << std::endl;
}