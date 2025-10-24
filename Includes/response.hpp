#ifndef __RESPONSE_H_
#define __RESPONSE_H_


#include "HttpRequest.hpp"

#define BUFFER_SIZE 100000

/*
* StatusLineData: Data containing the information used by status line
* @HttpVersion: Http protocol version used to communicate with
* @statusCode: The response code to be send back to the client
* @reasonPhrase: A phrase describing the statusCode.
*/

class HttpRequest;

struct StatusLineData {
    std::string HttpVersion;
    e_StatusCode statusCode;
    std::string reasonPhrase;
};

/*
* strcut header: The header for the http response message
* @status_line: First line of the response message, contain:
*               HTTP_version SP status-code SP reason-phrase
*               Example: <HTTP/1.1 200 OK>
* @field_line: The other information send with the response message, contain:
*              field-name ":" OWS field-value OWS
*              Example: <Content-Type: text/html>
*/

struct Header {
    StatusLineData status_line;
    std::map<std::string, std::string> field_line;
};

/*
* struct chunked_body: contain the chuncked message body response
*                      if the transfer-encoding used
* @length: the size of the chunked data
* @content: the actual content data
*/

struct ChunkedBodyData {
    int length;
    std::string content;
};

/*
* Response: Class contain the full response message to be send
* @transferEncoding: a boolean indicating if the message body should be chunked or not
* @method: The method requested by the client {GET, POST, DELETE,...}
* @header: A header for the http response message
* @body: The body that will be send, if transfer-encoding not specified
* @chunkedBody: The body that will be send, if the transfer-encoding was specified
*/

class Response {
    bool transferEncoding;
    std::string method;
    Header responseHeader;
    std::string body;
    std::string path;
    std::string CGI_Header;
    std::list<ChunkedBodyData> chunkedBody;
    ServerConfig *currentServer;
    Location *currentLocation;
    size_t index;

    public:
        Response();
        void setMethod(std::string );
        void set_Server(ServerConfig *);
        void setHeader(Header );
        void addToBody(const char*, size_t size);
        void execute_method();
        void Get();
        void Delete();
        void handle_directorys(e_StatusCode&, std::string&, Location*);
        void delete_directory(e_StatusCode&, std::string&, Location*);
        std::string getResponse();
        void setField_line(std::map<std::string, std::string>&);
        void setPath(std::string _path);
        void fill_CGI_Header(std::string );
        void update_CGI_Header(std::string);

        // getter
        std::string getPath(void);
        Location* getCurrentRoute(void);
        std::string get_CGI_Header(void) const;
        size_t getIndex(void) const;

        // setter
        void setCurrentLocation(Location *loc);
        void setStatusCode(int statusCode);
        void setIndex(size_t);

};

void getDataFromRequest(HttpRequest request, Response &response);
std::string getTimeOftheDay(void);
bool pathExists(std::string path, struct stat *);
bool FileR_OK(std::string path);
std::string getReasonPhrase(e_StatusCode statusCode);
std::string makeBodyResponse(Location *currentLocation, int statusCode, std::map<int, std::string>&,std::string path);
void fillFieldLine(std::map<std::string, std::string> &field_line, std::string contentType, std::string contentLength);
std::string getMIME(std::string path);
bool isCGI(std::string, Location *);
std::string getCGI(std::string);        
std::string readFile(std::string);
Location* getCurrentLocation(std::string oldPath, ServerConfig *currentServer);
std::vector<std::string> split(const std::string &s, char delimiter);
std::string buildPath(std::string URI, std::string path, std::string);
bool methodAllowed(const Location* location, const std::string& method);
void listDirectory(const std::string& path, std::vector<std::string>&);
bool isDirectoryEmpty(const std::string& path);
void executeCGI(std::string outFile, char* args[]);
void readCGI(std::string filename, Response& response);
bool parseCGIheader(char*, size_t , Response& );
std::string get_body_chunk(std::ifstream& file_stream, Response& response);
#endif
