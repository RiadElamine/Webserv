
#include "../../Includes/Response/response.hpp"
#include "../../Includes/utils.hpp"
//handle redirection in the config file 

Response::Response() {
    responseHeader.status_line.HttpVersion = "HTTP/1.1";
    header_sent = false;
    stream.offset = 0;
    is_method_executed = false;
    cgi = false;
}

Response::~Response() {
    stream.file_stream.close();
}

bool Response::is_header_sent() const {
    return header_sent;
}

void Response::set_cgi(bool val) {
    is_cgi = val;
}

void Response::set_header_sent(bool val) {
    header_sent = val;
}

Response& Response::operator=(const Response&) {
    return *this;
}

void Response::setStatusCode(int statusCode)
{
    responseHeader.status_line.statusCode = (e_StatusCode)statusCode;
    responseHeader.status_line.reasonPhrase = getReasonPhrase((e_StatusCode)statusCode);
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

void Response::set_offset(size_t pos) {
   stream.offset = pos;
}

size_t Response::get_offset(void) const {
   return stream.offset;
}

bool Response::open_stream(std::string& file_path) {
    stream.file_stream.open(file_path.c_str(), std::ios_base::in | std::ios_base::binary);
    
    if (!stream.file_stream.is_open())
        return false;
    stream.file_stream.seekg(stream.offset, std::ios::beg);
    return true;
}

bool Response::is_cgi_strem_open() const {
    return stream.file_stream.is_open();
}

std::string Response::Read_chunks(size_t size) {
    std::string ret;

    if (!stream.file_stream.is_open()) {
        std::cout << "read from the body" << std::endl;
        if (stream.offset >= body.length())
            return "";
        ret = body.substr(stream.offset, size);
        stream.offset += size;
    }

    else if (stream.file_stream.eof()) {
        return "";
    }
    else {
        char *buffer = new char[size];
        stream.file_stream.read(buffer, size);
        ret = std::string(buffer, stream.file_stream.gcount());
        delete[] buffer;
    }
    return ret;
}

size_t Response::calculate_content_length() {
    std::streampos current = stream.file_stream.tellg(); // start of body
    stream.file_stream.seekg(0, std::ios::end);          // end of file
    std::streampos end = stream.file_stream.tellg();
    stream.file_stream.seekg(current, std::ios::beg);    // restore position
    size_t size = static_cast<size_t>(end - current);
    return (size - 1);
}

std::string Response::getHeader()
{
    // std::string header;
    std::stringstream header;
    std::map<std::string, std::string>::iterator tmp;

    header << responseHeader.status_line.HttpVersion << " " << responseHeader.status_line.statusCode << " " << responseHeader.status_line.reasonPhrase << "\r\n";

    for(std::map<std::string, std::string>::iterator it = responseHeader.field_line.begin(); it != responseHeader.field_line.end(); ++it) {
        if (cgi && to_lower(it->first) == "content-length")
        {
            tmp = it;
            header << it->first << ": " << calculate_content_length() << "\r\n";
        }
        else
            header << it->first << ": " << it->second << "\r\n";
    }
    if (cgi) {
        std::stringstream ss;
        ss << calculate_content_length();
        responseHeader.field_line[tmp->first] = ss.str();
    }

    header << "\r\n";
    return header.str();
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
        // fillFieldLine(responseHeader.field_line, "text/html", ss.str());
    }
}

void Response::execute_method() {
    // execute methods only once

    if (is_method_executed || is_cgi || method == "POST")
        return ;
    else 
        is_method_executed = true;

    if (method != "GET" && method != "DELETE" && method != "POST")
        return make_response(true, Not_Implemented);

    struct stat info;

    if (!pathExists(path, &info))
        // respond with 404 code status
        return make_response(true, Not_Found);
    else if (!methodAllowed(currentLocation, method))
        // statusCode = Method_Not_Allowed;
        return make_response(true,  Method_Not_Allowed);

    else if (method == "GET")
        return (this->Get(info));
    else if (method == "DELETE")
        return (this->Delete(info));

}

std::string get_old_path(std::string& path, std::string& root) {
    size_t index = path.find(root);
    if (index == std::string::npos)
        return "";
    index += root.length();
    std::string ret = path.substr(index, path.length() - index);
    if (ret[0] == '/')
        ret = ret.substr(1);
    return ret;
}

void Response::fillFieldLine(std::string content_type, std::string content_length) {
    responseHeader.field_line["Date"] = getTimeOftheDay();
    if (responseHeader.status_line.statusCode == Moved_Permanently) {
        std::string location = get_old_path(path, currentLocation->root);
        if (location.empty())
            location = "http://localhost:8080/";
        else
            location = "http://localhost:8080/" + location + "/";
        responseHeader.field_line["Location"] =  location;
    }
    responseHeader.field_line["Content-Type"] = content_type;
    responseHeader.field_line["Content-Length"] = content_length;
    responseHeader.field_line["Connection"] = "close";
    responseHeader.field_line["Server"] = "WebServer/1.1.0";
}

void Response::make_response(bool is_error, e_StatusCode statusCode, bool is_autoindex) {
    size_t content_length;
    std::string content_type;
    std::stringstream ss;

    responseHeader.status_line.statusCode = statusCode;
    // std::cout << "path: " << path << std::endl;

    if (is_error) {
        std::string error_page_path = currentServer->error_pages[(int)statusCode];
        if (!error_page_path.empty() && open_stream(error_page_path)) {
            // set stream to the page error
            stream.offset = 0;
            content_length = calculate_content_length();
            content_type = getMIME(error_page_path);
        }
        else
        {
            // create a generic build error page
            ss << "<!DOCTYPE html>\n<html>\n<head>\n<title>" << (int) statusCode << " " << getReasonPhrase(statusCode);
            ss << "</title>\n</head>\n<body>\n<center><h1>" << (int) statusCode << " " << getReasonPhrase(statusCode);
            ss << "</h1></center>\n<hr><center>WebServer/1.0</center>\n</body>\n</html>" ;
            body = ss.str();
            content_length = body.length();
            content_type = "text/html";
        }
    } else if (is_autoindex) {
        std::cout <<  "autoindex" << std::endl;
        std::vector<std::string> entries;
        listDirectory(path, entries);

        ss << "<!DOCTYPE html>\n<html>\n<head>\n<meta charset='UTF-8'>\n<title>Index of " << get_old_path(path, currentLocation->root) << "</title>\n";
        ss << "<style>\nbody { font-family: monospace; padding: 20px; }\ntable { border-collapse: collapse; width: 100%; }\n";
        ss << "th, td { padding: 8px 12px; border-bottom: 1px solid #ddd; }th { text-align: left; background: #f2f2f2; }";
        ss << "a { text-decoration: none; color: #0066cc; }a:hover { text-decoration: underline; }</style>\n";
        ss << "</head>\n<body>\n<h2>Index of " + get_old_path(path, currentLocation->root) + "</h2>\n<table>\n<tr><th>Name</th><th>Type</th></tr>\n";
        
        for (size_t i = 0; i < entries.size(); i++) {
            std::string name = entries[i];
            std::string link = "<a href='" + name + "'>" + name + "</a>";
            std::string type = (name[name.size() - 1] == '/') ? "Directory" : "File";
            ss << "<tr><td>" + link + "</td><td>" + type + "</td></tr>\n";
        }
        ss << "</table>\n</body>\n</html>\n";
        body = ss.str();
        content_type = "text/html";
        content_length = body.length();
    } else {
        // set stream to the
        std::cout << "path: " << path << std::endl;
        stream.offset = 0;
        if (!open_stream(path))
            throw std::runtime_error("Can't set the path of the stream file");
        else {
            content_length = calculate_content_length();
            content_type = getMIME(path);
        }
    }

    ss.clear();
    ss.str("");
    ss << content_length;
    fillFieldLine(content_type, ss.str());
}

void Response::Get(struct stat& info) {

    if (info.st_mode & S_IFDIR)
        // handle direcotry
        handle_directorys(currentLocation);
    else if (info.st_mode & S_IFREG)
        // handle reqular file
        return make_response(false, OK);
    else
        // Unknown file type
        return make_response(true, Internal_Server_Error);
}

void Response::Delete(struct stat& info) {

    if (info.st_mode & S_IFDIR)
         // handle direcotry
         delete_directory();
     else if (info.st_mode & S_IFREG) {
         // handle reqular file
         size_t ret = std::remove(path.c_str());
         if (ret)
             return make_response(true, Forbidden);
         else
             return make_response(true, No_Content);
     }
     else
         return make_response(true, Internal_Server_Error);
}

void Response::delete_directory() {

    if (path[path.length() - 1] != '/') {
        return make_response(true, Moved_Permanently);
    }
    if (!isDirectoryEmpty(path)) {
        return make_response(true, Conflict);
    }
    else {
        size_t ret = rmdir(path.c_str());
        if (ret != 0)
            return make_response(true, Forbidden);
        else
            return make_response(true, No_Content);
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

void Response::handle_directorys(Location *currentLocation) {

    if (path[path.length() - 1] != '/')
        return make_response(true, Moved_Permanently);

    std::string rePath = path + currentLocation->index;
    if (!currentLocation->index.empty() && FileR_OK(rePath))
    {
        path = rePath;
        return make_response(false, OK);
    }
    
    else {
        if (!currentLocation->autoindex)
            return make_response(true, Forbidden);
        else
            return make_response(false, OK, true);
    }
}
