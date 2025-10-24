#include "../Includes/response.hpp"

//the reason of using ifstream instead of file descriptor, is lssek not allowed in the subject

std::string get_body_chunk(std::ifstream& file_stream, Response& response) {
    char buffer[BUFFER_SIZE];
    
    file_stream.seekg(response.getIndex());
    if (file_stream.eof())
        return "";
    int size = file_stream.readsome(buffer, BUFFER_SIZE);
    std::cout << "size read: " << size << std::endl;
    std::cout << "eof: " << file_stream.eof() << std::endl;
    std::cout << "fail: " << file_stream.fail() << std::endl;
    std::cout << "buffer: " << std::string(buffer, size) ;
    int current_pos = file_stream.tellg();
    if (current_pos == -1)
        throw std::runtime_error("tellg function failed");
    response.setIndex(current_pos);
    return std::string(buffer, size);
}
