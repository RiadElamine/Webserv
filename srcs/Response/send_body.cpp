#include "../../Includes/Response/response.hpp"

//the reason of using ifstream instead of file descriptor, is lssek not allowed in the subject

// this function isn't optimized
//std::string get_body_chunk(std::string& file_name, Response& response) {
//    std::ifstream stream(file_name.c_str(), std::ios_base::in | std::ios_base::binary);
//    char buffer[BUFFER_SIZE];
//
//    // if there is any error during the process of the file, the server should send 501 internal server error code.
//    if  (!stream.is_open())
//        throw std::runtime_error("Can't open file {get_body_chunk function} line 10");
//    size_t pos = response.getIndex();
//    stream.seekg(pos);
//    if (stream.fail())
////        throw std::runtime_error("seekg failed: position out of range");
//        return "";
//
//    stream.read(buffer, BUFFER_SIZE);
//    int size = stream.gcount();
//
//    response.setIndex(pos + size);
//    std::string ret(buffer, size);
//
//    stream.close();
//    return ret;
//    return "";
//}

// that the alternative

//std::string get_body_chunk(Response& response) {
//
//}
