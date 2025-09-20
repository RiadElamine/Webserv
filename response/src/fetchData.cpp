#include "../includes/response.hpp"

void getDataFromRequest(HttpRequest request, Response response){
    Header copyHeader;

    response.setMethod(request.getMethod());
    copyHeader.status_line.HttpVersion = "http/1.1";
    copyHeader.status_line.statusCode = request.getStatusCode();
    //implement a function to get the reason phrase depend on the status code
    copyHeader.status_line.reasonPhrase = "";
    response.setHeader(copyHeader);
}