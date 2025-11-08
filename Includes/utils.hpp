#ifndef UTILS_HPP
#define UTILS_HPP

#include "./Response/response.hpp"
#include "./CGI/Cgi.hpp"


// Function to check if a path is a CGI script based on location settings
bool isCGI(std::string path, Location *currentLocation);

// Function to match the longest valid Route from a path
size_t matchNB(const std::string& Route, const std::string& path);

// Function to get the location from the server config based on Route path
Location* getCurrentLocation(std::string oldPath, ServerConfig *currentServer);

std::string buildPath(std::string path, std::string root, std::string Route) ;


void getDataFromRequest(HttpRequest &request, Response &response);
std::string getTimeOftheDay(void);
bool pathExists(std::string path, struct stat *);
bool FileR_OK(std::string path);
std::string getReasonPhrase(e_StatusCode statusCode);
std::string makeBodyResponse(Location *currentLocation, int statusCode, std::map<int, std::string>&,std::string path);
void fillFieldLine(std::map<std::string, std::string> &field_line, std::string contentType, std::string contentLength);
std::string getMIME(std::string path);
std::string getCGI(std::string);        
std::string readFile(std::string);
std::vector<std::string> split(const std::string &s, char delimiter);
bool methodAllowed(const Location* location, const std::string& method);
void listDirectory(const std::string& path, std::vector<std::string>&);
bool isDirectoryEmpty(const std::string& path);
void readCGI(std::string filename, Response& response);
bool parseCGIheader(std::string& header, char *buffer , size_t buffer_size, Response& response);
std::string get_body_chunk(std::string&, Response&);
std::string to_lower(const std::string& str);
#endif // UTILS_HPP