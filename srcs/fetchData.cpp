#include "../Includes/response.hpp"
#include "../Includes/HttpRequest.hpp"

void getDataFromRequest(HttpRequest request, Response &response){
    Header copyHeader;

    response.setMethod(request.getMethod());
    response.setPath(request.get_path());
    std::cout << request.get_path() << std::endl; //Rebuild path before assign it to the response object
    copyHeader.status_line.HttpVersion = "HTTP/1.1";
    copyHeader.status_line.statusCode = (e_StatusCode) request.getStatusCode();
    copyHeader.status_line.reasonPhrase = getReasonPhrase(copyHeader.status_line.statusCode);
    response.setHeader(copyHeader);
}
