#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>

class HttpRequest {
    private:
        std::string method;
        std::string uri;
        std::map<std::string, std::string> headers;
        int contentLength;
        std::string chunked;
        std::string path;
        std::string query;
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
    public:
        HttpRequest();
        int  parse_request();
        void method_valid();
        void uri_valid();
        void decode(std::string &value);
        void parse_headers(std::string& data);
        void parse_body(std::string& data);
        void printRequest() const;
        void handl_boundary(std::string& data, size_t boundary_pos,  std::ofstream& file);
        bool headers_complete() const {
            return flag_headers;
        }
        int getContentLength() const {
            return contentLength;
        }

        std::string getMethod() const {
            return method;
        }
        std::string RequestData;
};

#endif