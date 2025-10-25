#include "../Includes/response.hpp"

//size_t headerEnd(char *buffer, size_t buffer_size, Response& response){
//
//}

static std::string trim(const std::string &s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

size_t get_index_of(std::string& str, char match, size_t start, size_t end) {
    for (;start < end; start++){
        if (str[start] == match)
            return start;
    }
    return std::string::npos;
}

std::string to_lower(const std::string& str) {
    std::string ret;

    for (size_t i = 0; i < str.size(); ++i) {
        ret += std::tolower(str[i]);
    }
    return ret;
}


// still need to check if CRLF not found, and EOF ecountered
bool form_header(char *buffer, size_t size, std::string& header) {
    bool ret(false);

    header.append(std::string(buffer, size));    
    size_t index = header.find("\r\n\r\n");

    if (index != std::string::npos) {
        header = header.substr(0, index);
        header.append("\r\n");
        ret = true;
    }
    return ret;
}



void make_field_line(std::map<std::string, std::string>& filed_line, std::string& header) {
    size_t end(0);
    size_t start(0);

    while (end < header.size()) {
        //won't discard the first line
        end = header.find("\r\n", end);
        if (end == std::string::npos)
            break ;

        size_t mid = get_index_of(header, ':', start, end);
        if (mid == std::string::npos)
            throw std::runtime_error("Missing colon (:) for headers");

        std::string key = trim(header.substr(start, (mid - start)));
        std::string value = trim(header.substr(mid + 1, (end - mid)));
        
        end += 2;
        start = end;
        if (!key.empty())
            filed_line[key] = value;
    }
}


size_t match_only(std::string str, char match, size_t start, size_t end, size_t occurence) {
    size_t count(0);
    size_t index;

    for (;start < end; ++start) {
        if (str[start] == match) {
            index = start;
            ++count;
        }
    }
    if (count == occurence)
        return index;
    return std::string::npos;
}

void set_statusCode(std::string& statusCode, Response& response) {
    size_t len = statusCode.size();
    size_t index = match_only(statusCode, ' ', 0, len, 1);
    if (index == std::string::npos && !isdigit(statusCode[len - 1]))
        throw std::runtime_error("status code and the reason phrase should have only one space between them");
    std::stringstream ss(statusCode);
    int val ;
    ss >> val;
    if (val < 100 || val > 599)
        throw std::runtime_error("Invalid range of status code");

    response.setStatusCode(val);
}

bool parseCGIheader(std::string& header, char *buffer , size_t buffer_size, Response& response) { //
    if (!form_header(buffer, buffer_size, header))
        return false;

    std::map<std::string, std::string> field_line;
    bool statusCode_found(false);
    bool valid_header(false);
    std::map<std::string, std::string>::iterator tmp;
    response.setIndex(header.size() + 2);

    try {
        make_field_line(field_line, header);

        for (std::map<std::string, std::string>::iterator it = field_line.begin(); it != field_line.end(); ++it) {
            std::string key = to_lower(it->first);
            if (key == "status") {
                set_statusCode(it->second, response);
                statusCode_found = true;
                tmp = it;

            }
            if (key == "content-type" || key == "location")
                valid_header = true;
        }

        if (!valid_header)
            throw std::runtime_error("Invalid Header, content-type, and location not provided");

        if (!statusCode_found)
            response.setStatusCode(200);
        else
            field_line.erase(tmp);

        response.setField_line(field_line);
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        response.setStatusCode(502);
    }

    return true;
}

int main() {
    Response response;
    std::string Header;

    int fd = open("/Users/mac/Webserv/cgi_test", O_RDONLY);
    char buffer[BUFFER_SIZE];
    int size;

    while ((size = read(fd, buffer, BUFFER_SIZE)) > 0) {
        if (parseCGIheader(Header, buffer, size, response))
            break ;
    }
    close(fd);
//    std::cout << response.getResponse() ;
    std::string file_name("/Users/mac/Webserv/cgi_test");
    std::string chunk;
//
    while (!(chunk = get_body_chunk(file_name, response)).empty())
    {
        std::cout  << chunk ;
    }

    return (0);
}
