#include "../includes/response.hpp"

void getDataFromRequest(HttpRequest request, Response response){
    Header copyHeader;

    response.setMethod(request.getMethod());
    response.setPath(request.get_path());
    copyHeader.status_line.HttpVersion = "http/1.1";
    copyHeader.status_line.statusCode = request.getStatusCode();
    copyHeader.status_line.reasonPhrase = getReasonPhrase(copyHeader.status_line.statusCode);
    response.setHeader(copyHeader);
}
