#ifndef __RESPONSE_H_
#define __RESPONSE_H_

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <cctype>
#include <map>
#include <list>
#include <ctime>
#include <stdexcept>
#include "status_code.hpp"

/*
* StatusLineData: Data containing the information used by status line
* @HttpVersion: Http protocol version used to communicate with
* @statusCode: The response code to be send back to the client
* @reasonPhrase: A phrase describing the statusCode.
*/

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
* HttpRequest: A demo class for the http request, used to silence the program errors
*              Until the HttpRequest Class API provided.
*/

class HttpRequest {
    public:
        HttpRequest(){};
        std::string getMethod() { return "GET"; };
        std::string getHttpVersion(){ return "http/1.1"; };
        e_StatusCode getStatusCode() { return OK; };
        std::string get_path() { return "/home/oel-asri/Webserv/response/index.html"; } ///home/oel-asri/Webserv/response/index.html
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
    std::string path;
    std::string body;
    std::list<ChunkedBodyData> chunkedBody;
    public:
        Response();
        void setMethod(std::string );
        void setPath(std::string );
        void setHeader(Header );
        void execute_method();
        void Get();
        void Delete();
        std::string getResponse();
        std::string getMe() {return method;}
};

void getDataFromRequest(HttpRequest request, Response &response);
std::string getTimeOftheDay(void);
bool pathExists(std::string path);
bool FileR_OK(std::string path);
std::string getReasonPhrase(e_StatusCode statusCode);
std::string makeBodyResponse(std::string reasonPhrase, int statusCode, std::string path);
void fillFieldLine(std::map<std::string, std::string> &field_line, std::string contentType, std::string contentLength);
std::string getMIME(std::string path);
bool isCGI(std::string);
std::string getCGI(std::string);        
std::string readFile(std::string);
#endif
