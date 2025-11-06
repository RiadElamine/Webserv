#ifndef __RESPONSE_H_
#define __RESPONSE_H_

/*
* StatusLineData: Data containing the information used by status line
* @HttpVersion: Http protocol version used to communicate with
* @statusCode: The response code to be send back to the client
* @reasonPhrase: A phrase describing the statusCode.
*/

#include "../brain.hpp"


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

struct fileStream {
    size_t offset;
    std::ifstream file_stream;
};

/*
* Response: Class contain the full response message to be send
* @transferEncoding: a boolean indicating if the message body should be chunked or not
* @method: The method requested by the client {GET, POST, DELETE,...}
* @header: A header for the http response message
* @body: The body that will be send, if transfer-encoding not specified
* @chunkedBody: The body that will be send, if the transfer-encoding was specified
*/

class Response : virtual public brain {
    std::string method;
    Header responseHeader;
    std::string body;
    fileStream stream;
    bool header_sent;
    bool is_method_executed;
    bool is_Moved_Permanently;
    bool is_index;
    bool is_data_fetched;
    bool to_open;

    public:
        Response();
        ~Response();
        Response& operator=(const Response&);
        void setMethod(std::string );
        void set_Server(ServerConfig *);
        void setHeader(Header );
        void addToBody(const char*, size_t size);
        void execute_method();
        void Get(struct stat& );
        void Delete(struct stat& );
        void handle_directorys();
        void delete_directory();
        std::string getResponse();
        void setField_line(std::map<std::string, std::string>&);
        void setPath(std::string _path);
        std::string Read_chunks(size_t size);
        size_t calculate_content_length();
        void make_response(bool is_error, e_StatusCode statusCode, bool is_autoindex = false);
        void fillFieldLine(std::string content_type, std::string content_length);
        bool process_path();
        void fetch_data_from_request(e_StatusCode, std::string);
        void handle_redirection();

        // getter
        std::string getPath(void);
        Location* getCurrentRoute(void);
        size_t get_offset(void) const;
        bool is_cgi_strem_open() const;
        bool is_header_sent() const;
        std::string getHeader();
        size_t get_status_code();

        // setter
        void setCurrentLocation(Location *loc);
        void set_is_fetched_data(bool);
        void set_method_executed(bool);
        void set_to_open(bool);
        void setStatusCode(int statusCode);
        void set_header_sent(bool val);
        void set_offset(size_t pos);
        bool open_stream(std::string& file_path); // if this return false, the server should response with 500

};


#endif
