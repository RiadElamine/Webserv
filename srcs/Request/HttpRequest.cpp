#include "../../Includes/Request/HttpRequest.hpp"
#include "../../Includes/utils.hpp"

HttpRequest::HttpRequest() {
    contentLength = 0;
    chunked = "";
    flag_headers = 0;
    body_complete = false;
    chunk_size = 0;
    boundary = "";
    flag_body = 0;
    flag_boundary = 0;
    need_boundary = false;
    code_status = 200;
    filename = "";
}


void HttpRequest::method_valid()
{
    if (method =="GET" || method == "POST" ||method == "DELETE")
        return;
    for(size_t i = 0; i < method.size(); i++)
    {
        if (!isupper(method[i]))
            return make_delay(400);
    }
}

std::string HttpRequest::remove_dot_segments(std::string path) {
    std::string result;
    result.reserve(path.size());

    size_t i = 0;
    while (i < path.size()) {
        if (path[i] == '/' && path[i+1] == '/')
            return make_delay(400),"";
        if (path.substr(i, 3) == "/./") {
            result += '/';
            i += 2;
        } else if (path.substr(i, 4) == "/../") {
            if (!result.empty()) {
                size_t last_slash = result.find_last_of('/', result.size() - 2);
                result.resize(last_slash + 1);
            }
            i += 3;
        } else if (path.substr(i, 2) == "..") {
            if (i + 2 == path.size()) {
                if (!result.empty()) {
                    size_t last_slash = result.find_last_of('/', result.size() - 2);
                    result.resize(last_slash + 1);
                }
                i += 2;
            } else {
                result += path[i++];
            }
        } else if (path.substr(i, 1) == ".") {
            if (i + 1 == path.size()) {
                i++;
            } 
            else 
                result += path[i++];
        } else {
            if (*(result.end() - 1) == '/' && path[i] == '/')
            {
                i++;
                continue;
            }
            result += path[i++];
        }
    }
    return result;
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

void HttpRequest::Route_valid()
{
    if (path[0] != '/' || path.empty() || path.size() > 8000)
        return make_delay(400);
    std::string character("-._~:/?#@[]!$&'()*+,;=%");
    for(size_t i = 0; i < path.size(); i++)
    {
        if (!std::isalnum(path[i]) && (character.find(path[i]) == std::string::npos))
            return make_delay(400);
    }
    if (path.find('?') != std::string::npos)
    {
        size_t pos = path.find('?');
        std::string query = path.substr(pos + 1);
        path.erase(pos);
        if (query.empty() || query[query.size() - 1] == '#')
            return make_delay(400);
        decode(query);
        size_t start = 0;
        while (true) {
            size_t equal_pos = query.find('=', start);
            if (equal_pos == std::string::npos) break;
            std::string key = query.substr(start, equal_pos - start);
            size_t and_pos = query.find('&', equal_pos);
            std::string value = query.substr(equal_pos + 1, and_pos - equal_pos - 1);
            params[key] = value;
            if (and_pos == std::string::npos) break;
            start = and_pos + 1;
        }
    }
    decode(path);
}

void HttpRequest::set_status(int status)
{
    code_status = status;
    body_complete = true;
}

void HttpRequest::make_delay(int status)
{
    int i = 0;
    while( i< 1500000)i++;
    return set_status(status);
}

void HttpRequest::parse_headers(std::string& data) {
    int i = 0;
    i = data.find(' ');
    method = data.substr(0, i);
    method_valid();
    data.erase(0, i+1);
    i = data.find(' ');
    path = data.substr(0, i);
    Route_valid();
    data.erase(0, i+1);
    i = data.find("\r\n");
    if(data.substr(0, i).empty() || (data.substr(0, i) != "HTTP/1.1"))
        return make_delay(400);
    data.erase(0, i+2);
    size_t header_end = data.find("\r\n\r\n");
    std::string header_block = data.substr(0, header_end);
    for (size_t start = 0; start < header_block.size(); ) {
        size_t line_end = header_block.find("\r\n", start);

        std::string line = header_block.substr(start, line_end - start);
        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            std::string key = line.substr(0, colon_pos);
            if (key.empty() || key.find_first_not_of(" \t") != 0 || key.find_last_not_of(" \t") != key.size() - 1)
                return make_delay(400);
            std::string value = line.substr(colon_pos + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            headers[key] = value;
        }
        if (line_end > header_block.size())break; 
        start = line_end + 2;
    }
    data.erase(0, header_end+4);
    if (headers.find("Transfer-Encoding") != headers.end() && headers.find("Content-Length") != headers.end())
        return make_delay(400);
    if (headers.find("Content-Length") != headers.end()) {
        contentLength = std::atol(headers["Content-Length"].c_str());
        if (contentLength <= 0) 
            return make_delay(400);
        if (contentLength > currentServer->client_max_body_size) 
            return make_delay(413);
    } 
    if (headers.find("Transfer-Encoding") != headers.end()) {
        if (headers["Transfer-Encoding"] == "chunked")
            chunked = "chunked";
        else
            return make_delay(400);
    }
    if (headers.find("Content-Type") != headers.end() && headers["Content-Type"].find("multipart/form-data;") != std::string::npos) {
        size_t boundary_pos = headers["Content-Type"].find("boundary=");
        if (boundary_pos == std::string::npos || headers["Content-Type"].substr(boundary_pos + 9).empty()) 
            return make_delay(400);
        if (boundary_pos != std::string::npos) {
            boundary = headers["Content-Type"].substr(boundary_pos + 9);
        } 
    }
    if (!headers["Transfer-Encoding"].empty() && !headers["Content-Length"].empty())
        return make_delay(400);
    if (!headers["Content-Length"].empty()) {
        contentLength = std::atol(headers["Content-Length"].c_str());
        if (contentLength <= 0) 
            return make_delay(400);
        if (contentLength > currentServer->client_max_body_size) 
            return make_delay(413);
    } 
    if (!headers["Transfer-Encoding"].empty()) {
        if (headers["Transfer-Encoding"] == "chunked")
            chunked = "chunked";
        else
            return make_delay(400);
    }
    if (!headers["Content-Type"].empty() && headers["Content-Type"].find("multipart/form-data;") != std::string::npos) {
        size_t boundary_pos = headers["Content-Type"].find("boundary=");
        if (boundary_pos == std::string::npos || headers["Content-Type"].substr(boundary_pos + 9).empty()) 
            return make_delay(400);
        if (boundary_pos != std::string::npos) {
            boundary = headers["Content-Type"].substr(boundary_pos + 9);
        } 
    }

    flag_headers = true;
}

void HttpRequest::check(std::string& data, size_t pos)
{
    if (data.find(boundary, pos) != std::string::npos)
        set_status(400);
    else
        need_boundary = true;
}

void HttpRequest::handl_boundary(std::string& data, size_t boundary_pos) {
    if (flag_boundary == 0 && flag_body == 0) {
        size_t start_pos = boundary_pos + boundary.size() + 2;
        data.erase(0, start_pos);
        size_t pos = data.find("filename=", 0);
        if (pos != std::string::npos) {
                std::string name;
                std::string content;
                size_t name_pos = data.find("filename=");
                if (name_pos != std::string::npos) {
                    name_pos += 10;
                    size_t end_name_pos = data.find('"', name_pos);
                    if (end_name_pos != std::string::npos) {
                        name = data.substr(name_pos, end_name_pos - name_pos);
                        form_data["filename"] = name;
                        create_file(1);
                        data.erase(0, end_name_pos + 3);
                        data.erase(0, data.find("\r\n\r\n") + 4);
                        flag_boundary = 1;
                    } else 
                        return check(data, pos);
                } else 
                    return check(data, pos);
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
        code_status = 201;
        if (data.find("--"+boundary+"--\r\n" , boundary.size()+6) != std::string::npos && data.substr(0, data.find("--"+boundary+"--\r\n" , boundary.size()+6)).find(boundary) == std::string::npos) {
            data.clear();
            body_complete = true;
        }
    }
}

void HttpRequest::inchunk_body(std::string &data)
{
   Location *it = getCurrentLocation(path, currentServer); 
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
            if (boundary.empty())
                set_status(201);
            return;
        }
        pos = chunk_size_end + 2;
        if (pos + chunk_size + 2 > data.size()) {
            need_boundary = true;
            chunk_size = 0;
            return;
        }
    }
    if (boundary.empty() || isCGI(path, it))
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
    if (contentLength > currentServer->client_max_body_size) {
        remove(filename.c_str());
        return set_status(413);
    }
    if (data.substr(pos, 2) != "\r\n") {
        file.close();
        return set_status(400);
    }
    pos += 2;
    chunk_size = 0;
    if (pos > 0) {
        data.erase(0, pos);
    }
}

std::string random_string(size_t length) {
    static const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(charset) - 2); 

    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += charset[dis(gen)];
    }
    return result;
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


void HttpRequest::create_file(int flag)
{
    Location* it = getCurrentLocation(path, currentServer);
    if (std::find(it->methods.begin(), it->methods.end(), method) == it->methods.end()) 
        return  set_status(405);
    std::string build_pat = buildPath(path, it->root);
    struct stat buffer;
    if (stat(build_pat.c_str(), &buffer) != 0 || !S_ISDIR(buffer.st_mode) || !(buffer.st_mode & S_IWUSR)) 
        return  set_status(500);
    std::string ext;
    if (!boundary.empty() && !form_data["filename"].empty()) {
        const std::string& orig_filename = form_data["filename"];
        size_t dot_pos = orig_filename.find_last_of('.');
        if (dot_pos != std::string::npos && dot_pos < orig_filename.length() - 1)
            ext = orig_filename.substr(dot_pos);
    } else
        ext = get_mime_type(); 
    const int MAX_ATTEMPTS = 1000;
    for (int i = 1; i <= MAX_ATTEMPTS; ++i) {
        std::string candidate = build_pat + "/upload_" + random_string(8) + ext;
        if (stat(candidate.c_str(), &buffer) != 0) {
            if (flag == 1)
                form_data["filename"] = std::move(candidate);
            else
                filename = std::move(candidate);
            return;
        }
    }
}




void HttpRequest::parse_body(std::string& data) {
    Location *it = getCurrentLocation(path, currentServer);
    if (filename == "" && boundary.empty())
        create_file(0);
    if (!chunked.empty())
    {
        inchunk_body(data);
    }
    if (contentLength > 0 && chunked.empty())
    {
        if (boundary.empty() || isCGI(path, it))
        {
            std::ofstream file(filename.c_str(), std::ios::binary | std::ios::app);   
            // size_t bytesToWrite = std::min(static_cast<size_t>(contentLength), data.size());
            size_t bytesToWrite = (contentLength > data.size()) ? data.size() : contentLength;
            file.write(data.c_str(), bytesToWrite);
            // std::cout << "awdi " << bytesToWrite << std::endl;
            contentLength -= bytesToWrite;
            data.erase(0, bytesToWrite);
            if (contentLength <= 0) {
                file.close();
                return set_status(201);
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