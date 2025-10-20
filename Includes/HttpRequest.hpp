#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "ConfigFile.hpp"
#include "response.hpp"
#include <random>

class HttpRequest {
    private:
        std::string method;
        std::string uri;
        std::map<std::string, std::string> headers;
        size_t contentLength;
        std::string chunked;
        std::string path;
        std::map<std::string, std::string> query_params;
        std::string version;
        bool flag_headers;
        bool body_complete;
        int j;
        size_t chunk_size;
        std::string boundary;
        int flag_body;
        int flag_boundary;
        std::map<std::string, std::string> form_data;
        std::string inchunk;
        bool need_boundary;
        ServerConfig *server;
        int code_status;
        std::string RequestData;
        void check(std::string& data, size_t pos);
        std::string filename;
        std::string remove_dot_segments(std::string path);
    public:
        HttpRequest();
        int  parse_request(char* buffer, ssize_t n);
        void method_valid();
        void uri_valid();
        void decode(std::string &value);
        void parse_headers(std::string& data);
        void parse_body(std::string& data);
        void printRequest() const;
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
        std::string getVersion() const {
            return version;
        }
        std::map<std::string, std::string> getQueryParams() const {
            return query_params;
        }
        int getStatusCode() { return code_status; };
        void inchunk_body(std::string& data);
        std::string getPath() { return path; }
        ServerConfig *getServer() const {
            return server;
        }
        void setServer(ServerConfig *srv) {
            server = srv;
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