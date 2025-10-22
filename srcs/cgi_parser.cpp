#include "../Includes/response.hpp"
#define BUFFER_SIZE 100000

//size_t headerEnd(char *buffer, size_t buffer_size, Response& response){
//
//}

void remake_header(std::string& Header, size_t index, Response& response) {
    std::string sub = Header.substr(0, index);
    response.update_CGI_Header(sub);
}


bool form_header(char *buffer, size_t size, Response& response) {
    bool ret(false);

    std::string tmp(buffer, size);
    response.fill_CGI_Header(tmp);
    
    std::string Header = response.get_CGI_Header();
    size_t index = Header.find("\r\n\r\n");

    if (index != std::string::npos) {
        remake_header(Header, index, response);
        ret = true;
    }
    return ret;
}

bool parseCGIheader(char *buffer, size_t buffer_size, Response& response __attribute__ ((unused))) { //
    if (!form_header(buffer, buffer_size, response))
        return false;
    
    
    
    return true;
}


//int main() {
////    std::ifstream file("/Users/oel-asri/Kingsave/Webserv/cgi_test", std::ifstream::in);
//    Response response;
//
//    int fd = open("/Users/oel-asri/Kingsave/Webserv/cgi_test", O_RDONLY);
//    char buffer[BUFFER_SIZE];
//    size_t size;
//
//    while ((size = read(fd, buffer, BUFFER_SIZE)) > 0) {
////        std::cout << "size: " << size << std::endl;
//        parseCGIheader(buffer, size, response);
//    }
//
////    if (file.eof())
////        std::cout << "Encouter EOF" << std::endl;
//
//    close(fd);
////    file.close();
//
//    return (0);
//}
