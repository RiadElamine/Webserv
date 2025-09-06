
#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <algorithm>
#include <unistd.h>
#include <netdb.h>
#include <utility>
#include <set>

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
            std::set<std::pair<std::string, uint16_t> >  listens;
            size_t client_max_body_size;
            std::string global_root;
            std::string global_index;
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
    Location location;
    int indexOfErrorPages;
    int indexOfRedircat;
    size_t length;
    std::ifstream ConfigFileStream;
    ServerConfig server;
    size_t has_other_block;
    
    void Initialize();

    // first detect key
    void DispatchParser();
    
    // keys : this defaul in server
    void verifyServerKeyword();
    void verifyDelimiter(CharSymbol char_symbol);
    void ParseErrorPages();
    void ParseGlobalRoot();
    void ParseListen();
    void ParseClientMaxBodySize();
    void ParseIndexGlobal();

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


    std::map<std::string, bool> directiveFlags;
    std::map<std::string, void (ConfigFile::*)()> funcs;

    std::map<std::string, bool> directiveFlagsLocation;
    std::map<std::string, void (ConfigFile::*)()> funcs_location;

    void reset();
    void resetLocation();


	void fill_server_defaults();


    ConfigFile();

    public:
    ConfigFile(int argc, char **argv);
    ServerConfig *parse();
};