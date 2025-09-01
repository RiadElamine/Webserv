#include "../Includes/HttpRequest.hpp"

#include <sstream>
#include <string>   
#include <algorithm>
#include <cctype>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <cstring>
#include <unistd.h>


HttpRequest::HttpRequest() {
    contentLength = 0;
    chunked = "";
    path = "";
    query = "";
    flag_headers = 0;
    body_complete = false;
    j = 0;
    chunk_size = 0;
    boundary = "";
    flag_body = 0;
    flag_boundary = 0;
    need_boundary = false;
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
    if (method =="GET" || method == "POST" || method == "HEAD" ||method == "DELETE")
        return;
    for(size_t i = 0; i < method.size(); i++)
    {
        if (!isupper(method[i]))
        {
            std::cout << "not upper" << std::endl;
            return;
        }
    }
    std::cout << "invalid method" << method << std::endl;
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
        std::cout << "error" << std::endl;
        return;
    }
    std::string character("-._~:/?#@[]!$&'()*+,;=%");
    for(size_t i = 0; i < uri.size(); i++)
    {
        if (!std::isalnum(uri[i]) && (character.find(uri[i]) == std::string::npos))
        {
            std::cout << "error-" <<std::endl;
            return;
        }
    }
    if (uri.find('?') != std::string::npos)
    {
        size_t pos = uri.find('?');
        path = uri.substr(0, pos);
        std::string query = uri.substr(pos + 1);
        decode(query);
        size_t start = 0;
        while (true) {
            size_t equal_pos = query.find('=', start);
            if (equal_pos == std::string::npos) break;
            std::string key = query.substr(start, equal_pos - start);
            size_t and_pos = query.find('&', equal_pos);
            std::string value = query.substr(equal_pos + 1, and_pos - equal_pos - 1);
            query_params[key] = value;
            std::cout << key << " " << query << " " << value << std::endl;
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
        std::cout << "error " <<version.empty() <<std::endl;
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
            std::cerr << "No boundary found in content-type header." << std::endl;
            throw std::runtime_error("No boundary found in content-type header.");
        }
        if (boundary_pos != std::string::npos) {
            boundary = headers["Content-Type"].substr(boundary_pos + 9);
        } 
    }
}

void HttpRequest::handl_boundary(std::string& data, size_t boundary_pos,  std::ofstream& file) {
    (void)file;
    // if (data.find(boundary, boundary_pos + boundary.size()) == std::string::npos && flag_boundary == 0 && flag_body == 0) {
    //     std::cout << "No boundary found in data." << std::endl;
    //     std::ofstream ll("no_boundary.txt", std::ios::binary | std::ios::app);
    //     ll << data;
    //     need_boundary = true;
    //     return;
    // }
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
                    std::cerr << "Invalid filename format." << std::endl;
                    return;
                }
                size_t content_start = data.find("\r\n\r\n", end_name_pos);
                if (content_start != std::string::npos) {
                    content_start += 4;
                    size_t content_end = data.find("\r\n", content_start);
                    if (content_end != std::string::npos) {
                        content = data.substr(content_start, content_end - content_start);
                    } else {
                        std::cerr << "Boundary not found after file content." << std::endl;
                        return;
                    }
                } else {
                    std::cerr << "Content start not found." << std::endl;
                    return;
                }
                data.erase(0, data.find(boundary));
                form_data[name] = content;
                std::cout << "----- " << name << " " << content << std::endl;
                if (data.find("--\r\n" , boundary.size()) != std::string::npos && data.find("--\r\n" , boundary.size()) < data.find("\r\n")) {
                    std::cout << "Boundary ------------found, removing 1 it from data." << std::endl;
                    data.erase(0, data.find("--\r\n") + 4);
                    flag_body = 1;
                }
                return;
            }
            else {
                std::cerr << "No filename found in multipart/form-data." << std::endl;
                return;
            }
        }
        if (pos < data.find("\r\n")) {
            std::cout << "Handling multipart/form-data without filename." << std::endl;
                std::string name;
                std::string content;
                size_t name_pos = data.find("filename=");
                if (name_pos != std::string::npos) {
                    name_pos += 10;
                    size_t end_name_pos = data.find('"', name_pos);
                    if (end_name_pos != std::string::npos) {
                        std::cout << "======" << std::endl;
                        name = data.substr(name_pos, end_name_pos - name_pos);
                        form_data["filename"] = name+ "txt";
                        data.erase(0, end_name_pos + 3);
                        data.erase(0, data.find("\r\n\r\n") + 4);
                        flag_boundary = 1;
                    } else {
                        std::cerr << "Invalid filename format." << std::endl;
                        return;
                    }
                } else {
                    std::cerr << "No filename found in multipart/form-data." << std::endl;
                    return;
                }
        }
    }
    if (flag_boundary == 1) {
        std::cout << "Boundary already handled, writing to file." << std::endl;
        std::ofstream file(form_data["filename"], std::ios::binary | std::ios::app);
        if (data.find(boundary) == std::string::npos) {
            file.write(data.c_str(), data.size());
            std::cout << "** No boundary found, writing remaining data to file." << std::endl;
            std::ofstream oo("uup.txt", std::ios::binary | std::ios::app);
            oo << data << "{visord}";
            need_boundary = true;
            data.clear();
            return;
        }
        file << data.substr(0, data.find(boundary) - 4);
        std::cout << "** Boundary found, writing data to file." << std::endl;
        flag_boundary = 0;
        file.close();
        // data.erase(0, data.find(boundary) + boundary.size() + 2);
        // std::cout << "[" <<data << "]" << std::endl;
        if (data.find("--\r\n" , boundary.size()) != std::string::npos && data.find("--\r\n" , boundary.size()) < data.find("\r\n")) {
            std::cout << "Boundary -------- found, removing it from data." << std::endl;
            data.clear();
            flag_body = 1;
        }
    }
}


// void HttpRequest::parse_body(std::string& data) {
    
//     std::ofstream file("body.txt", std::ios::binary | std::ios::app);
//     if (!file.is_open()) {
//         return;
//     }
//     if (!boundary.empty() && contentLength > 0) {
//         size_t boundary_pos = data.find(boundary);
//         handl_boundary(data, boundary_pos, file);
//         return;
//     }
//     if (!boundary.empty() && !chunked.empty()) {
//         std::istringstream iss;
//         size_t pos = 0;
//         if (chunk_size == 0) {
//             size_t chunk_size_end = data.find("\r\n", pos);
//             if (chunk_size_end == std::string::npos) {
//                 file.close();
//                 return;
//             }
//             std::string size_line = data.substr(pos, chunk_size_end - pos);
//             iss.str(size_line);
//             iss.clear(); 
//             iss >> std::hex >> chunk_size;
//             if (chunk_size == 0 && flag_body == 1) {
//                 data.erase(0, chunk_size_end + 2);
//                 file.close();
//                 std::cout << "Chunk size is 0, closing file." << std::endl;
//                 return;
//             }
//             pos = chunk_size_end + 2;
//             if (pos + chunk_size + 2 > data.size()) {
//                 std::cout << "**** Not enough data for chunk." << std::endl;
//                 need_boundary = true;
//                 chunk_size = 0;
//                 return;
//             }
//         }
//         inchunk.append(data.c_str() + pos, chunk_size);
//         size_t boundary_pos = inchunk.find(boundary);
//         // size_t boundary_end = inchunk.find(boundary, boundary_pos + boundary.size());
//         // std::cout << "Boundary position: " << boundary_pos << std::endl;
//         // std::cout << "Boundary end position: " << boundary_end << std::endl;
//         // size_t inchunkpos = inchunk.find("--\r\n", boundary_pos);
//         // std::cout << "Inchunk position: " << inchunkpos << std::endl;
//         // if (boundary_end != std::string::npos || (boundary_pos != std::string::npos && inchunkpos != std::string::npos)) {
//             handl_boundary(inchunk, boundary_pos, file);
//         // }
//         pos += chunk_size;
//         std::cout << "Chunk size: " << chunk_size << std::endl;
//         if (data.substr(pos, 2) != "\r\n") {
            
//             throw std::runtime_error("Invalid chunk format - missing CRLF");
//         }
//         pos += 2;
//         chunk_size = 0;
//         if (pos > 0) {
//             data.erase(0, pos);
//         }
//     }

//     if (contentLength > 0 && boundary.empty()) {
//         size_t bytesToWrite = std::min(static_cast<size_t>(contentLength), data.size());
//         file.write(data.c_str(), bytesToWrite);
//         contentLength -= bytesToWrite;
//         data.erase(0, bytesToWrite); 
//         if (contentLength <= 0) {
//             file.close();
//             return;
//         }
//     }
//     else if (chunked == "chunked" && boundary.empty()) {
//         std::cout << "Chunked transfer encoding detected." << std::endl;
//         std::istringstream iss;
//         size_t pos = 0;
//         bool done = false;
//         if (chunk_size == 0) {
//             size_t chunk_size_end = data.find("\r\n", pos);
//             if (chunk_size_end == std::string::npos) {
//                 file.close();
//                 return;
//             }
//             std::string size_line = data.substr(pos, chunk_size_end - pos);
//             iss.str(size_line);
//             iss.clear(); 
//             iss >> std::hex >> chunk_size;
//             if (chunk_size == 0) {
//                 done = true;
//                 data.erase(0, chunk_size_end + 2);
//                 file.close();
//                 return;
//             }
//             pos = chunk_size_end + 2;
//             if (pos + chunk_size + 2 > data.size()) {
//                 need_boundary = true;
//                 return;
//             }
//         }
//         file.write(data.c_str() + pos, chunk_size);
//         pos += chunk_size;
//         if (data.substr(pos, 2) != "\r\n") {
            
//             throw std::runtime_error("Invalid chunk format - missing CRLF");
//         }
//         pos += 2;
//         chunk_size = 0;
//         if (pos > 0) {
//             data.erase(0, pos);
//         }
//     }

// }

//v1
//----------------------------------------------------------------------------

// void HttpRequest::parse_body(std::string &data)
// {
//     (void)data;
//     if (!chunked.empty())
//     {
//         std::istringstream iss;
//         size_t pos = 0;
//         if (chunk_size == 0)
//         {
//             size_t chunk_size_end = data.find("\r\n", pos);
//             if (chunk_size_end == std::string::npos)
//             {
//                 return;
//             }
//             std::string size_line = data.substr(pos, chunk_size_end - pos);
//             std::cout <<  size_line  << std::endl;
//             iss.str(size_line);
//             iss.clear();
//             iss >> std::hex >> chunk_size;
//             if (chunk_size == 0 && flag_body == 1)
//             {
//                 data.erase(0, chunk_size_end + 2);
//                 std::cout << "Chunk size is 0, finishing body parsing." << std::endl;
//                 return;
//             }
//             pos = chunk_size_end + 2;
//             if (pos + chunk_size + 2 > data.size())
//             {
//                 std::cout << "**** Not enough data for chunk." << std::endl;
//                 need_boundary = true;
//                 chunk_size = 0;
//                 return;
//             }
//             std::cout<<"---- " << pos + chunk_size + 2 << " <= " << data.size() << std::endl;
//         }
//         inchunk.append(data.c_str() + pos, chunk_size);
//         // size_t boundary_pos = inchunk.find(boundary);
//         // if (!boundary.empty() && boundary_pos != std::string::npos)
//         // {
//         //     std::ofstream file(form_data["filename"], std::ios::binary | std::ios::app);
//         //     handl_boundary(inchunk, boundary_pos, file);
//         //     file.close();
//         // }
//         std::ofstream file("file", std::ios::binary | std::ios::app);
//         file << inchunk;
//         inchunk.clear();
//         pos += chunk_size;
//         if (data.substr(pos, 2) != "\r\n")
//         {
//             std::cout << "Invalid format - missing CRLF" << std::endl;
//             throw std::runtime_error("Invalid chunk format - missing CRLF");
//         }
//         pos += 2;
//         chunk_size = 0;
//         if (pos > 0)
//         {
//             data.erase(0, pos);
//         }
//     }

// }
//v2
//----------------------------------------------------------------------------

void HttpRequest::parse_body(std::string& data) {
    
    std::ofstream file("body.txt", std::ios::binary | std::ios::app);
    if (!file.is_open()) {
        return;
    }
    if (!boundary.empty() && contentLength > 0) {
        size_t boundary_pos = data.find(boundary);
        handl_boundary(data, boundary_pos, file);
        return;
    }
    if (!boundary.empty() && !chunked.empty()) {
        std::istringstream iss;
        size_t pos = 0;
        if (chunk_size == 0) {
            size_t chunk_size_end = data.find("\r\n", pos);
            if (chunk_size_end == std::string::npos) {
                file.close();
                return;
            }
            std::string size_line = data.substr(pos, chunk_size_end - pos);
            iss.str(size_line);
            iss.clear(); 
            iss >> std::hex >> chunk_size;
            if (chunk_size == 0 && flag_body == 1) {
                data.erase(0, chunk_size_end + 2);
                file.close();
                std::cout << "Chunk size is 0, closing file." << std::endl;
                return;
            }
            pos = chunk_size_end + 2;
            if (pos + chunk_size + 2 > data.size()) {
                std::cout << "**** Not enough data for chunk." << std::endl;
                need_boundary = true;
                chunk_size = 0;
                return;
            }
        }
        inchunk.append(data.c_str() + pos, chunk_size);
        size_t boundary_pos = inchunk.find(boundary);
        size_t boundary_end = inchunk.find(boundary, boundary_pos + boundary.size());
        std::cout << "Boundary position: " << boundary_pos << std::endl;
        std::cout << "Boundary end position: " << boundary_end << std::endl;
        size_t inchunkpos = inchunk.find("--\r\n", boundary_pos);
        std::cout << "Inchunk position: " << inchunkpos << std::endl;
        if (boundary_end != std::string::npos || (boundary_pos != std::string::npos && inchunkpos != std::string::npos)) {
            handl_boundary(inchunk, boundary_pos, file);
        }
        pos += chunk_size;
        if (data.substr(pos, 2) != "\r\n") {
            
            throw std::runtime_error("Invalid chunk format - missing CRLF");
        }
        pos += 2;
        chunk_size = 0;
        if (pos > 0) {
            data.erase(0, pos);
        }
    }
    if (contentLength > 0 && boundary.empty()) {
        size_t bytesToWrite = std::min(static_cast<size_t>(contentLength), data.size());
        file.write(data.c_str(), bytesToWrite);
        contentLength -= bytesToWrite;
        data.erase(0, bytesToWrite); 
        if (contentLength <= 0) {
            file.close();
            return;
        }
    }
    else if (chunked == "chunked" && boundary.empty()) {
        std::cout << "Chunked transfer encoding detected." << std::endl;
        std::istringstream iss;
        size_t pos = 0;
        bool done = false;
        if (chunk_size == 0) {
            size_t chunk_size_end = data.find("\r\n", pos);
            if (chunk_size_end == std::string::npos) {
                file.close();
                return;
            }
            std::string size_line = data.substr(pos, chunk_size_end - pos);
            iss.str(size_line);
            iss.clear(); 
            iss >> std::hex >> chunk_size;
            if (chunk_size == 0) {
                done = true;
                data.erase(0, chunk_size_end + 2);
                file.close();
                return;
            }
            pos = chunk_size_end + 2;
            if (pos + chunk_size + 2 > data.size()) {
                need_boundary = true;
                chunk_size = 0;
                return;
            }
        }
        file.write(data.c_str() + pos, chunk_size);
        pos += chunk_size;
        if (data.substr(pos, 2) != "\r\n") {
            
            throw std::runtime_error("Invalid chunk format - missing CRLF");
        }
        pos += 2;
        chunk_size = 0;
        if (pos > 0) {
            data.erase(0, pos);
        }
    }

}


int HttpRequest::parse_request() {

    if (RequestData.find("\r\n\r\n") == std::string::npos && !headers_complete()) {
        std::cout << "Incomplete headers, waiting for more data." << std::endl;
        return (body_complete);
    }
    std::cout <<"------------------" << std::endl;
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
        std::cout << headers_complete() << std::endl;
        std::cout << "After parsing headers, remaining data size: " << RequestData.size() << std::endl;
        if (method == "POST" && headers_complete()) {
            parse_body(RequestData);
        }
    }
    std::cout << "Final remaining data size: " << RequestData.size() << std::endl;
    return (body_complete);
}