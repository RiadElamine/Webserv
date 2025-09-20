#ifndef __RESPONSE_H_
#define __RESPONSE_H_

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <list>

/*
* StatusLineData: Data containing the information used by status line
* @HttpVersion: Http protocol version used to communicate with
* @statusCode: The response code to be send back to the client
* @reasonPhrase: A phrase describing the statusCode.
*/

struct StatusLineData {
    std::string HttpVersion;
    int statusCode;
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
    std::list<ChunkedBodyData> chunkedBody;
    public:
        Response();
        void setMethod(std::string );
        void setHeader(Header );
};

/*
* HttpRequest: A demo class for the http request, used to silence the program errors
*              Until the HttpRequest Class API provided.
*/

class HttpRequest {
    public:
        HttpRequest();
        std::string getMethod() { return ""; };
        std::string getHttpVersion(){ return ""; };
        int getStatusCode() { return 0; };
};

#endif
