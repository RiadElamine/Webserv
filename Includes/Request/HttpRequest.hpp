#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "../brain.hpp"

class HttpRequest : virtual public brain {
    private:
        std::string method;
        std::map<std::string, std::string> headers;
        size_t contentLength;
        std::string chunked;
        bool flag_headers;
        bool body_complete;
        size_t chunk_size;
        std::string boundary;
        int flag_boundary;
        std::map<std::string, std::string> form_data;
        std::string inchunk;
        bool need_boundary;
        int code_status;
        std::string RequestData;
        void check(std::string& data, size_t pos);
        std::string remove_dot_segments(std::string path);
        void set_status(int status);
        void method_valid();
        void Route_valid();
        void decode(std::string &value);
        void parse_headers(std::string& data);
        void parse_body(std::string& data);
        void handl_boundary(std::string& data, size_t boundary_pos);
        bool headers_complete() const;
        void create_file(int flag);
        void inchunk_body(std::string& data);
    public:
        HttpRequest();
        int  parse_request(char* buffer, ssize_t n);

        std::string getMethod() const;
        std::map<std::string, std::string> getHeaders() const;
        int getStatusCode()const;
        std::string getPath() { return path; }
        ServerConfig *getServer() const;
        std::string get_filename() const;
        std::string get_mime_type()const;
        void setStatusCode(int code_status);
};

#endif