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
        int flag_body;
        int flag_boundary;
        std::map<std::string, std::string> form_data;
        std::string inchunk;
        bool need_boundary;
        int code_status;
        std::string RequestData;
        void check(std::string& data, size_t pos);
        std::string remove_dot_segments(std::string path);
        void make_delay(int status);
    public:
        HttpRequest();
        int  parse_request(char* buffer, ssize_t n);
        void method_valid();
        void Route_valid();
        void decode(std::string &value);
        void parse_headers(std::string& data);
        void parse_body(std::string& data);
        void handl_boundary(std::string& data, size_t boundary_pos);
        bool headers_complete() const {
            return flag_headers;
        }
        int getContentLength() const {
            return contentLength;
        }

        std::string getMethod() const {
            return method;
        }
        std::map<std::string, std::string> getHeaders() const {
            return headers;
        }
        int getStatusCode() { return code_status; };
        void inchunk_body(std::string& data);
        std::string getPath() { return path; }
        ServerConfig *getServer() const {
            return currentServer;
        }
        void create_file(int flag);
        std::string get_filename() const {
            return filename;
        }
        std::string get_mime_type()const;
        void setStatusCode(int code_status) { this->code_status = code_status; }
        void set_status(int status);
};

#endif