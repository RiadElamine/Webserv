
#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>


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
            

enum CharSymbol
{
    OPEN_BRACKET   = '{',
    CLOSE_BRACKET  = '}',
    SEMICOLON    = ';',
};
class ConfigFile
{
    int tokenize[5];
    int tokenize_location[4];
    int tokenize_method_location[2];
    int tokenize_method[2];
    int index_of_t = 0;

    int index_of_lm = 0;
    int i = 0;
    bool is_location = false;
    bool check_final = false;
    bool check_final_location = false;

    void verifyServerKeyword(std::string &word);
    void verifyDelimiter(std::string &word, CharSymbol char_symbol);
    
    // first detect key
    // void detectkey(std::string	&word);
    void DispatchParser(const std::string &key);
    
    // keys : this defaul in server
    void ParseErrorPages(std::string	&word);
    void ParseRoot(std::string	&word);
    void ParsePort(std::string	&word);
    void ParseDomain(std::string	&word);

    //keys: this in location
    void Parselocationblock(std::string	&word);
    void verifyLocationPath(std::string	&word);
    void verifyDelimiterLocation(std::string &word, CharSymbol char_symbol);


    //if parselocationblock is good than call this fuction
    void DispatchParserLocation(const std::string& key);
    

    void ParseAutoindex(std::string	&word);
    void ParseMethods(std::string	&word);
    void ParseCGI(std::string	&word);
    void ParseUpload(std::string	&word);
    void ParseRedir(std::string	&word);
    void ParseLocationRoot(std::string	&word);
    void ParseIndex(std::string	&word);
    
    void (ConfigFile::*call)(std::string&);
    public:
    // std::vector<ServerConfig> servers;
    void parse(const std::string& file_path);
};