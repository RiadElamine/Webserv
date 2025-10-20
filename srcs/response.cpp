#include "../Includes/response.hpp"


//handle redirection in the config file 

Response::Response() {
    transferEncoding = false;
}

void Response::setStatusCode(int statusCode)
{
    responseHeader.status_line.statusCode = (e_StatusCode)statusCode;
}

void Response::setMethod(std::string _method) {
    method = _method;
}

void Response::setPath(std::string _path) {
    path = _path;
}

///
std::string Response::getPath(void) {
    return path;
}

void  Response::setCurrentLocation(Location *loc) { 
    currentLocation = loc; 
}

Location* Response::getCurrentRoute(void) { 
    return currentLocation; 
}
///

void Response::set_Server(ServerConfig *server) {
    currentServer = server;
}

void Response::addToBody(const char* chunk, size_t size) {
    body.append(chunk, size);
}

void Response::setField_line(std::map<std::string, std::string>& filed_line) {
    for(std::map<std::string, std::string>::iterator it = filed_line.begin(); it != filed_line.end(); ++it){
        responseHeader.field_line[it->first] = it->second;
    }
}
void Response::setHeader(Header copyHeader) {
    responseHeader.status_line.HttpVersion = copyHeader.status_line.HttpVersion;
    responseHeader.status_line.statusCode = copyHeader.status_line.statusCode;
    responseHeader.status_line.reasonPhrase = copyHeader.status_line.reasonPhrase;
    // check for the status from request parsing

    if (responseHeader.status_line.statusCode != OK) {
        std::stringstream ss;
        body = makeBodyResponse(NULL, \
                                responseHeader.status_line.statusCode, currentServer->error_pages, "");

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
    // respond with 501 not implemented

}

//modify path

void Response::Get() {
    e_StatusCode statusCode;
    std::string mime("");
    std::stringstream ss;
    struct stat info;
    std::string oldPath = path;

    Location *currentLocation = getCurrentLocation(path, currentServer);
    path = buildPath(currentLocation->URI, path, currentLocation->root);
    std::cout << "path: " << path << std::endl;
    if (!pathExists(path, &info)) {
        // respond with 404 code status
        statusCode = Not_Found;
        mime = "text/html";
        body = makeBodyResponse(currentLocation, statusCode, currentServer->error_pages, "");
    }
    else if (!methodAllowed(currentLocation, "GET")) {
        statusCode = Method_Not_Allowed;
        mime = "text/html";
        body = makeBodyResponse(currentLocation, statusCode, currentServer->error_pages, "");
    }
    else if (isCGI(path, currentLocation)) {
        Response response;

        char *args[3] = {
            (char*) "/Users/relamine/Desktop/cgi/cgi-bin/cgi/env/bin/python3",
            (char *) "/Users/relamine/Desktop/cgi/cgi-bin/cgi/getLatestNew.py",
            NULL
        };
        // std::string path_ = path.erase(0);
        // args[1] = (char *) path_.c_str();
        // args[2] = NULL;
        executeCGI("/Users/relamine/Desktop/cgi/CGI", args);
        readCGI("/Users/relamine/Desktop/cgi/CGI", response);
        responseHeader.status_line.statusCode = response.responseHeader.status_line.statusCode;
        responseHeader.status_line.reasonPhrase = response.responseHeader.status_line.reasonPhrase;
        responseHeader.status_line.HttpVersion = response.responseHeader.status_line.HttpVersion;
        setField_line(response.responseHeader.field_line);
        body = response.body;
        std::ostringstream ss;
        ss << body.size();
        responseHeader.field_line["Content-Length"] = ss.str();
        return ;
    }
    else if (info.st_mode & S_IFDIR) {
        // handle direcotry
        statusCode = UNITILAZE;
        handle_directorys(statusCode, mime, currentLocation);
    }
    else if (info.st_mode & S_IFREG) {
        // handle reqular file
        statusCode = OK;
        mime = getMIME(path);
        body = makeBodyResponse(currentLocation, statusCode, currentServer->error_pages, path);
    } else {
        mime = "text/html";
        statusCode = Internal_Server_Error;
        body = makeBodyResponse(currentLocation, statusCode, currentServer->error_pages, "");
    }

    responseHeader.status_line.statusCode = statusCode;
    responseHeader.status_line.reasonPhrase = getReasonPhrase(statusCode);
    if (statusCode == Moved_Permanently)
        responseHeader.field_line["Location"] =  "http://localhost:8080" + oldPath + "/";
    ss << body.length();
    ss.str();
    ss.clear();
    fillFieldLine(responseHeader.field_line, mime, ss.str());
}

void Response::Delete() {
    e_StatusCode statusCode;
    std::string mime("");
    std::stringstream ss;
    struct stat info;

    Location *currentLocation = getCurrentLocation(path, currentServer);
    // if (currentLocation->root[currentLocation->root.length() - 1] != '/')
    //     currentLocation->root += '/';
    path = buildPath(currentLocation->URI, path, currentLocation->root);

    if (!pathExists(path, &info)) {
        // respond with 404 code status
        statusCode = Not_Found;
        mime = "text/html";
        body = makeBodyResponse(currentLocation, statusCode, currentServer->error_pages, "");
    }
    else if (!methodAllowed(currentLocation, "DELETE")) {
        statusCode = Method_Not_Allowed;
        // mime = "text/html";
        body = makeBodyResponse(currentLocation, statusCode, currentServer->error_pages, "");
    }
    else if (info.st_mode & S_IFDIR) {
        // handle direcotry
        statusCode = UNITILAZE;
        delete_directory(statusCode, mime, currentLocation);
    }
    else if (info.st_mode & S_IFREG) {
        // handle reqular file
        size_t ret = std::remove(path.c_str());
        if (ret) {
            statusCode = Forbidden;
            mime = "text/html";
            body =  makeBodyResponse(currentLocation, statusCode, currentServer->error_pages, "");
        }
        else 
            statusCode = No_Content;
    } else {
        mime = "text/html";
        statusCode = Internal_Server_Error;
        body = makeBodyResponse(currentLocation, statusCode, currentServer->error_pages, "");
    }

    responseHeader.status_line.statusCode = statusCode;
    responseHeader.status_line.reasonPhrase = getReasonPhrase(statusCode);
    ss << body.length();
    ss.str();
    ss.clear();
    fillFieldLine(responseHeader.field_line, mime, ss.str());
}

void Response::delete_directory(e_StatusCode& statusCode, std::string& mime, Location* currentLocation __attribute__ ((unused))) {
    if (path[path.length() - 1] != '/') {
        statusCode = Moved_Permanently;
        mime = "text/html";
        body = makeBodyResponse(NULL, statusCode, currentServer->error_pages, "");
        return ;
    }
    if (!isDirectoryEmpty(path)) {
        statusCode = Conflict;
        mime = "text/html";
        body = makeBodyResponse(NULL, statusCode, currentServer->error_pages, "");
    }
    else {
        size_t ret = rmdir(path.c_str());
        if (ret != 0) {
            statusCode = Forbidden;
            mime = "text/html";
            body = makeBodyResponse(NULL, statusCode, currentServer->error_pages, "");
        }
        else
            statusCode = No_Content;
    }
}


std::string Response::getResponse() {
    std::string message;
    std::stringstream ss;

    ss << responseHeader.status_line.statusCode ;
    message += responseHeader.status_line.HttpVersion + " " + ss.str() + " " + responseHeader.status_line.reasonPhrase + "\r\n";
    for(std::map<std::string, std::string>::iterator it = responseHeader.field_line.begin(); it != responseHeader.field_line.end(); ++it) {
        message += it->first + ": " + it->second + "\r\n";
    }
    message += "\r\n" + body;
    ss.str("");
    ss.clear();
    return message;
}

void Response::handle_directorys(e_StatusCode& statusCode, std::string& mime, Location *currentLocation) {
    if (path[path.length() - 1] != '/') {
        statusCode = Moved_Permanently;
        mime = "text/html";
        body = makeBodyResponse(NULL, statusCode, currentServer->error_pages, "");

        return ;
    }
    std::string rePath = path + currentLocation->index;
    if (!currentLocation->index.empty() && FileR_OK(rePath)) {
        statusCode = OK;
        mime = getMIME(rePath);
        body = makeBodyResponse(NULL, statusCode, currentServer->error_pages, rePath);
    } else {
        if (!currentLocation->autoindex) {
            statusCode = Forbidden;
            mime = "text/html";
            body = makeBodyResponse(NULL, statusCode, currentServer->error_pages, "");
        } else {
            statusCode = OK;
            mime = "text/html";
            std::vector<std::string> entries;
            listDirectory(path, entries);

            body  = "<!DOCTYPE html>\n<html>\n<head>\n";
            body += "<meta charset='UTF-8'>\n";
            body += "<title>Index of " + path + "</title>\n";
            body += "<style>"
                    "body { font-family: monospace; padding: 20px; }"
                    "table { border-collapse: collapse; width: 100%; }"
                    "th, td { padding: 8px 12px; border-bottom: 1px solid #ddd; }"
                    "th { text-align: left; background: #f2f2f2; }"
                    "a { text-decoration: none; color: #0066cc; }"
                    "a:hover { text-decoration: underline; }"
                    "</style>\n";
            body += "</head>\n<body>\n";

            body += "<h2>Index of " + path + "</h2>\n";
            body += "<table>\n";
            body += "<tr><th>Name</th><th>Type</th></tr>\n";

            for (size_t i = 0; i < entries.size(); i++) {
                std::string name = entries[i];
                std::string link = "<a href='" + name + "'>" + name + "</a>";
                std::string type = (name[name.size() - 1] == '/') ? "Directory" : "File";

                body += "<tr><td>" + link + "</td><td>" + type + "</td></tr>\n";
            }

            body += "</table>\n";
            body += "</body>\n</html>\n";
        }
    }
}
