#include "../Includes/response.hpp"
#include "../Includes/HttpRequest.hpp"


std::string remakePath(HttpRequest request) {
    
}



void getDataFromRequest(HttpRequest request, Response &response){
    Header copyHeader;

    response.setMethod(request.getMethod());
    std::string path = remakePath(request); //Rebuild path before assign it to the response object
    response.setPath(path);
    std::cout << path << std::endl;
    copyHeader.status_line.HttpVersion = "HTTP/1.1";
    copyHeader.status_line.statusCode = (e_StatusCode) request.getStatusCode();
    copyHeader.status_line.reasonPhrase = getReasonPhrase(copyHeader.status_line.statusCode);
    response.setHeader(copyHeader);
}
