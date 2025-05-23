
#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>


class Location
{
    public:
        std::string root;
        std::string index;
        std::vector<std::string> methods;
        bool autoindex;
        std::map<int, std::string> redirect;
        std::string upload_store;
        std::string cgi_ext;
        std::string cgi_Path_Info;

        // Location();
};
    
class ServerConfig
{
        public:
            std::string host;
            int port;
            std::string server_name;
            long client_max_body_size;
            std::string global_root;
            std::map<int, std::string> error_pages;
            std::vector<Location> locations;

        // ServerConfig();
};
            

enum CharSymbol
{
    OPEN_BRACKET   = '{',
    CLOSE_BRACKET  = '}',
    SEMICOLON    = ';',
};

class ConfigFile
{
    int index_of_t = 0;
    int index_of_lm = 0;
    int i = 0;
    bool check_final = false;
    bool check_final_location = false;
    int check_semi = 0;
    std::string	word;
    void (ConfigFile::*call)();
    ServerConfig server;
    Location location;
    int indexOfErrorPages = 0;
    int indexOfRedircat = 0;
    
    // first detect key
    void DispatchParser();
    
    // keys : this defaul in server
    void verifyServerKeyword();
    void verifyDelimiter(CharSymbol char_symbol);
    void ParseErrorPages();
    void ParseGlobalRoot();
    void ParsePort();
    void ParseDomain();
    void ParseClientMaxBodySize();

    //keys: this in location
    void Parselocationblock();
    void verifyLocationPath();
    void verifyDelimiterLocation(CharSymbol char_symbol);


    //if parselocationblock is good than call this fuction
    void DispatchParserLocation();
    

    void ParseAutoindex();
    void ParseMethods();
    void ParseCGI();
    void ParseCGIPath();
    void ParseUpload();
    void ParseRedir();
    void ParseLocationRoot();
    void ParseIndex();
    
    std::string get_data(int max_data = 1);
    std::string get_method();
    void GetNameOfLocation();
    bool HasSpecialDelimiter(const std::string &data) const;
    bool L_HasSpecialDelimiter(const std::string &data) const;


    public:
    std::vector<ServerConfig> servers;
    void parse(const std::string& file_path);
};