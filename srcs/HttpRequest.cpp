#include "../Includes/HttpRequest.hpp"

HttpRequest::HttpRequest() {
    contentLength = 0;
    chunked = "";
    path = "";
    // query = "";
    flag_headers = 0;
    body_complete = false;
    j = 0;
    chunk_size = 0;
    boundary = "";
    flag_body = 0;
    flag_boundary = 0;
    need_boundary = false;
    code_status = 200;
    filename = "";
}
void HttpRequest::printRequest() const
{
    std::cout << "Method: " << method << "\n";
    std::cout << "URI: " << uri << "\n";
    std::cout << "Headers:\n";
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << it->first << ": " << it->second << "\n";
    }
}

void HttpRequest::method_valid()
{
    if (method =="GET" || method == "POST" ||method == "DELETE")
        return;
    for(size_t i = 0; i < method.size(); i++)
    {
        if (!isupper(method[i]))
        {
            code_status = 400;
            body_complete = true;
            return;
        }
    }
    code_status = 501;
    body_complete = true;
}

void HttpRequest::decode(std::string &value) {
    std::ostringstream decoded;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '%' && i + 2 < value.size()) {
            std::string hexStr = value.substr(i + 1, 2);
            if (isxdigit(hexStr[0]) && isxdigit(hexStr[1])) {
                int hex_val;
                std::istringstream(hexStr) >> std::hex >> hex_val;
                decoded << static_cast<unsigned char>(hex_val);
                i += 2;
            } else
                decoded << value[i];
        } else 
            decoded << value[i];
    }
    value = decoded.str();
}

void HttpRequest::uri_valid()
{
    if (uri[0] != '/' || uri.empty() || uri.size() > 8000)
    {
        code_status = 400;
        body_complete = true;
        return;
    }
    std::string character("-._~:/?#@[]!$&'()*+,;=%");
    for(size_t i = 0; i < uri.size(); i++)
    {
        if (!std::isalnum(uri[i]) && (character.find(uri[i]) == std::string::npos))
        {
            code_status = 400;
            body_complete = true;
            return;
        }
    }
    if (uri.find('?') != std::string::npos)
    {
        size_t pos = uri.find('?');
        path = uri.substr(0, pos);
        std::string query = uri.substr(pos + 1);
        //test this
        if (query.empty() || query[query.size() - 1] == '#')
        {
            code_status = 400;
            body_complete = true;
            return;
        }
        decode(query);
        size_t start = 0;
        while (true) {
            size_t equal_pos = query.find('=', start);
            if (equal_pos == std::string::npos) break;
            std::string key = query.substr(start, equal_pos - start);
            size_t and_pos = query.find('&', equal_pos);
            std::string value = query.substr(equal_pos + 1, and_pos - equal_pos - 1);
            query_params[key] = value;
            if (and_pos == std::string::npos) break;
            start = and_pos + 1;
        }
    }
    else
        path = uri;
    decode(path);
}



void HttpRequest::parse_headers(std::string& data) {
    int i = 0;
    i = data.find(' ');
    method = data.substr(0, i);
    method_valid();
    data.erase(0, i+1);
    i = data.find(' ');
    uri = data.substr(0, i);
    uri_valid();
    data.erase(0, i+1);
    i = data.find("\r\n");
    version = data.substr(0, i);
    if(version.empty() && (version != "HTTP/1.1"))
    {
        code_status = 400;
        body_complete = true;
        return;
    }
    data.erase(0, i+2);
    size_t header_end = data.find("\r\n\r\n");
    std::string header_block = data.substr(0, header_end);
    for (size_t start = 0; start < header_block.size(); ) {
        size_t line_end = header_block.find("\r\n", start);

        std::string line = header_block.substr(start, line_end - start);
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            std::string value = line.substr(colon_pos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            headers[key] = value;
        }
        if (line_end > header_block.size())
            break; 
        start = line_end + 2;
    }
    data.erase(0, header_end+4);
    flag_headers = 1;
    if (headers.find("Content-Length") != headers.end()) {
        contentLength = std::atol(headers["Content-Length"].c_str());
        if (contentLength < 0) {
            code_status = 400;
            body_complete = true;
            return;
        }
        if (contentLength > server->client_max_body_size) {
            std::cout << "Invalid Content-Length value: " << headers["Content-Length"] << std::endl;
            code_status = 413;
            body_complete = true;
            return;
        }
    } else {
        contentLength = 0;
    }
    if (headers.find("Transfer-Encoding") != headers.end() && headers["Transfer-Encoding"] == "chunked") {
        chunked = "chunked";
    } else {
        chunked = "";
    }
    if (headers.find("Content-Type") != headers.end() && headers["Content-Type"].find("multipart/form-data;") != std::string::npos) {
        size_t boundary_pos = headers["Content-Type"].find("boundary=");
        if (boundary_pos == std::string::npos) {
            code_status = 400;
            body_complete = true;
            return;
        }
        if (boundary_pos != std::string::npos) {
            boundary = headers["Content-Type"].substr(boundary_pos + 9);
        } 
    }
}

void HttpRequest::check(std::string& data, size_t pos)
{
    if (data.find(boundary, pos) != std::string::npos)
    {
        code_status = 400;
        body_complete = true;
    }
    else
        need_boundary = true;
}

void HttpRequest::handl_boundary(std::string& data, size_t boundary_pos) {
    if (flag_boundary == 0 && flag_body == 0) {
        size_t start_pos = boundary_pos + boundary.size() + 2;
        data.erase(0, start_pos);
        size_t pos = data.find("filename=", 0);
        if (pos  > data.find("\r\n")) {
            std::string name;
            std::string content;
            size_t name_pos = data.find("name="); 
            if (name_pos != std::string::npos) {
                name_pos += 6;
                size_t end_name_pos = data.find('"', name_pos);
                if (end_name_pos != std::string::npos) {
                    name = data.substr(name_pos, end_name_pos - name_pos);
                } else {
                    check(data, pos);
                    return;
                }
                size_t content_start = data.find("\r\n\r\n", end_name_pos);
                if (content_start != std::string::npos) {
                    content_start += 4;
                    size_t content_end = data.find("\r\n", content_start);
                    if (content_end != std::string::npos) {
                        content = data.substr(content_start, content_end - content_start);
                    } else {
                        check(data, pos);
                        return;
                    }
                } else {
                    check(data, pos);
                    return;
                }
                data.erase(0, data.find(boundary));
                form_data[name] = content;
                if (data.find("--\r\n" , boundary.size()) != std::string::npos && data.find("--\r\n" , boundary.size()) < data.find("\r\n")) {
                    data.erase(0, data.find("--\r\n") + 4);
                    flag_body = 1;
                    body_complete = true;
                }
                return;
            }
            else {
                check(data, pos);
                return;
            }
        }
        if (pos < data.find("\r\n")) {
                std::string name;
                std::string content;
                size_t name_pos = data.find("filename=");
                if (name_pos != std::string::npos) {
                    name_pos += 10;
                    size_t end_name_pos = data.find('"', name_pos);
                    if (end_name_pos != std::string::npos) {
                        name = data.substr(name_pos, end_name_pos - name_pos);
                        form_data["filename"] = name;
                        data.erase(0, end_name_pos + 3);
                        data.erase(0, data.find("\r\n\r\n") + 4);
                        flag_boundary = 1;
                    } else {
                        check(data, pos);
                        return;
                    }
                } else {
                    check(data, pos);
                    return;
                }
        }
    }
    if (flag_boundary == 1) {
        std::ofstream file(form_data["filename"], std::ios::binary | std::ios::app);
        if (data.find(boundary) == std::string::npos) {
            file.write(data.c_str(), data.size());
            data.clear();
            return;
        }
        file << data.substr(0, data.find(boundary) - 4);
        flag_boundary = 0;
        file.close();
        if (data.find("--\r\n" , boundary.size()) != std::string::npos && data.find("--\r\n" , boundary.size()) < data.find("\r\n")) {
            data.clear();
            // flag_body = 1;
            body_complete = true;
            code_status = 201;
        }
    }
}

void HttpRequest::inchunk_body(std::string &data)
{
    std::ofstream file(filename.c_str(), std::ios::binary | std::ios::app);
    std::istringstream iss;
    size_t pos = 0;
    if (chunk_size == 0) {
        size_t chunk_size_end = data.find("\r\n", pos);
        if (chunk_size_end == std::string::npos) {
            file.close();
            need_boundary = true;
            return;
        }
        std::string size_line = data.substr(pos, chunk_size_end - pos);
        iss.str(size_line);
        iss.clear(); 
        iss >> std::hex >> chunk_size;
        if (chunk_size == 0) {
            data.erase(0, chunk_size_end + 2);
            file.close();
            if (!boundary.empty())
                code_status = 201;
            return;
        }
        pos = chunk_size_end + 2;
        if (pos + chunk_size + 2 > data.size()) {
            need_boundary = true;
            chunk_size = 0;
            return;
        }
    }
    if (boundary.empty() || cgi())
    {
        if (!file.is_open())
            file.open(filename.c_str(), std::ios::binary | std::ios::app);
        file.write(data.c_str() + pos, chunk_size);
    }
    else if (!boundary.empty())
    {
        inchunk.append(data.c_str() + pos, chunk_size);
        size_t boundary_pos = inchunk.find(boundary);
        size_t boundary_end = inchunk.find(boundary, boundary_pos + boundary.size());
        size_t inchunkpos = inchunk.find("--\r\n", boundary_pos);
        if (boundary_end != std::string::npos || (boundary_pos != std::string::npos && inchunkpos != std::string::npos)) {
            handl_boundary(inchunk, boundary_pos);
        }
    }
    pos += chunk_size;
    contentLength += chunk_size;
    if (contentLength > server->client_max_body_size) {
        std::cout << "File size exceeded: " << contentLength << " bytes" << std::endl;
        remove(filename.c_str());
        code_status = 413;
        body_complete = true;
        return;
    }
    if (data.substr(pos, 2) != "\r\n") {
        file.close();
        code_status = 400;
        body_complete = true;
        return;
    }
    pos += 2;
    chunk_size = 0;
    if (pos > 0) {
        data.erase(0, pos);
    }
}

std::string generate_random_string(size_t length) {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = sizeof(charset) - 1;
    std::string random_string;
    random_string.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        random_string += charset[rand() % max_index];
    }
    return random_string;
}

std::string HttpRequest::get_mime_type() const{
    static std::map<std::string, std::string> mime_types;
    mime_types["text/html"] = ".html";
    mime_types["text/css"] = ".css";
    mime_types["application/javascript"] = ".js";
    mime_types["application/json"] = ".json";
    mime_types["image/png"] = ".png";
    mime_types["image/jpeg"] = ".jpg";
    mime_types["image/gif"] = ".gif";
    mime_types["image/svg+xml"] = ".svg";
    mime_types["text/plain"] = ".txt";
    mime_types["application/xml"] = ".xml";
    mime_types["application/pdf"] = ".pdf";
    mime_types["application/zip"] = ".zip";
    mime_types["application/octet-stream"] = ".bin";


    std::string ext = headers.find("Content-Type") != headers.end() ? headers.at("Content-Type") : "";
    return mime_types.find(ext) != mime_types.end() ? mime_types.at(ext) : ".bin";
}

void HttpRequest::create_file()
{
    struct  stat buffer;

    for (std::vector<Location>::iterator it = server->locations.begin(); it != server->locations.end(); ++it)
    {
        if (path == it->URI || (it->URI == path + "/"))
        {
            //---------------------------------------
            if (it->methods.end() == std::find(it->methods.begin(), it->methods.end(), method))
            {
                code_status = 405;
                body_complete = true;
                return;
            }
            if (stat(it->upload_store.c_str(), &buffer) != 0)
            {
                code_status = 500;
                body_complete = true;
                return;
            }
            else
            {
                std::cout << "Upload store directory exists: " << it->upload_store << std::endl;
                int i = 1;
                while (true)
                {
                    filename = it->upload_store + "/upload_" + generate_random_string(i) + get_mime_type();
                    if (stat(filename.c_str(), &buffer) != 0)
                        break;
                    i++;
                }
                
            }
            //---------------------------------------
        }
    }    
}

bool HttpRequest::cgi()
{
    return true;
}



void HttpRequest::parse_body(std::string& data) {
    if (filename == "")
        create_file();
    if (!chunked.empty())
    {
        inchunk_body(data);
    }
    if (contentLength > 0)
    {
        if (boundary.empty() || cgi())
        {
            std::ofstream file(filename.c_str(), std::ios::binary | std::ios::app);        
            size_t bytesToWrite = std::min(static_cast<size_t>(contentLength), data.size());
            file.write(data.c_str(), bytesToWrite);
            contentLength -= bytesToWrite;
            data.erase(0, bytesToWrite); 
            if (contentLength <= 0) {
                file.close();
                body_complete = true;
                code_status = 201;
                return;
            }
        }
        else{
            size_t boundary_pos = data.find(boundary);
            handl_boundary(data, boundary_pos);
        }
    }
}


int HttpRequest::parse_request(char* buffer, ssize_t n) {
    RequestData.append(buffer, n);

    if (RequestData.find("\r\n\r\n") == std::string::npos && !headers_complete()) {
        return (body_complete);
    }
    while (!RequestData.empty())
    {
        if (need_boundary == true)
        {
            need_boundary = false;
            break;
        }
        if (!headers_complete()) {
            parse_headers(RequestData);
        }
        if (((method== "GET" || method == "DELETE") && headers_complete()) || body_complete)
        {
            body_complete = true;
            RequestData.clear();
            break;
        }
        if (method == "POST" && headers_complete()) {
            parse_body(RequestData);
        }

    }
    return (body_complete);
}