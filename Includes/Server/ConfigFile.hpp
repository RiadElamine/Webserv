
#pragma once

#include "../Common.hpp"

// #define max_size_of_file 1048576 // 1MB
#define max_size_of_file 141800 // 1.4KB

class Location
{
    public:
        std::string Route;
        std::string root;
        std::string index;
        std::vector<std::string> methods;
        bool autoindex;
        std::map<int, std::string> redirect;
        std::map<std::string, std::string> cgi_Path_Info;

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
    private:
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
        std::vector<ServerConfig> *servers;
        std::string extension_cgi;
        // check state of file
        void checkFile(int argc, char **argv);
        // 
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
        void checkDuplicateRoutes(std::string &path_name);
        void verifyLocationPath();
        void verifyDelimiterLocation(CharSymbol char_symbol);
        //if parselocationblock is good than call this fuction
        void DispatchParserLocation();
        //
        void ParseAutoindex();
        void ParseMethods();
        void ParseCGIPath();
        void ParseRedir();
        void ParseLocationRoot();
        void ParseIndex();
        //
        std::string get_data(int max_data = 1);
        std::string get_data_location (int max_data = 1);
        void addMethod(const std::string &method);
        void get_method();
        // helpers
        std::map<std::string, bool> directiveFlags;
        std::map<std::string, void (ConfigFile::*)()> funcs;
        std::map<std::string, bool> directiveFlagsLocation;
        std::map<std::string, void (ConfigFile::*)()> funcs_location;
        // reset
        void reset();
        void resetLocation();
        // default values
        void fill_server_defaults();
    public:
        ConfigFile(int argc, char **argv);
        std::vector<ServerConfig> *parse();
};
