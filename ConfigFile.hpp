
#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <unistd.h>

class Location
{
    public:
        std::string URI;
        std::string root;
        std::string index;
        std::vector<std::string> methods;
        bool autoindex;
        std::map<int, std::string> redirect;
        std::string upload_store;
        std::string cgi_ext;
        std::string cgi_Path_Info;

        void reset();
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

        void reset();
};
            

enum CharSymbol
{
    OPEN_BRACKET   = '{',
    CLOSE_BRACKET  = '}',
    SEMICOLON    = ';',
};

class ConfigFile
{
    int index_of_t;
    int index_of_lm;
    size_t i;
    int check_semi;
    std::string	word;
    void (ConfigFile::*call)();
    ServerConfig server;
    Location location;
    int indexOfErrorPages;
    int indexOfRedircat;
    size_t length;
    
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
    void checkDuplicateURIs(std::string &path_name);
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
    std::string get_data_location (int max_data = 1);
    void addMethod(const std::string &method);
    void get_method();
    bool HasSpecialDelimiter(const std::string &data) const;
    bool L_HasSpecialDelimiter(const std::string &data) const;


    std::map<std::string, bool> directiveFlags;
    std::map<std::string, void (ConfigFile::*)()> funcs;

    std::map<std::string, bool> directiveFlagsLocation;
    std::map<std::string, void (ConfigFile::*)()> funcs_location;

    void reset();
    void resetLocation();


	void fill_server_defaults();

    public:
    std::vector<ServerConfig> servers;
    ConfigFile();
    void parse(const std::string& file_path);
};