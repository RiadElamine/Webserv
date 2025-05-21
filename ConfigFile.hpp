
#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>


enum CharSymbol
{
    OPEN_BRACKET   = '{',
    CLOSE_BRACKET  = '}',
    SEMICOLON    = ';',
};

// class Location
// {
//     public:
//         std::string root;
//         std::string index;
//         std::vector<std::string> methods;
//         bool autoindex;
//         std::string redirect;
//         std::string upload_store;
//         std::string cgi_ext;
//         std::string cgi_Path_Info;

//         Location();
// };

// class ServerConfig
// {
//     public:
//         std::string host;
//         int port;
//         std::string server_name;
//         std::string root;
//         size_t client_max_body_size;
//         std::map<int, std::string> error_pages;
//         std::vector<Location> locations;

//         ServerConfig();
// };

// class ConfigFile
// {
//         void verifyServerKeyword(std::string &word, bool &this_server);
//         void verifyDelimiter(std::string &word, int &curly_brac_s, bool &this_server);
//     public:
//         // std::vector<ServerConfig> servers;
//         void parse(const std::string& file_path);
// };

class ConfigFile
{
        void verifyServerKeyword(std::string &word, bool &this_server);
        void verifyDelimiter(std::string &word, int &curly_brac_s, bool &this_server);
    public:
        // std::vector<ServerConfig> servers;
        void parse(const std::string& file_path);
};