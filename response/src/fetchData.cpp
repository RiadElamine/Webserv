#include "../includes/response.hpp"

void getDataFromRequest(HttpRequest request, Response response){
    Header copyHeader;

    response.setMethod(request.getMethod());
    response.set_path(request.get_path());
    copyHeader.status_line.HttpVersion = "http/1.1";
    copyHeader.status_line.statusCode = request.getStatusCode();
<<<<<<< HEAD
    copyHeader.status_line.reasonPhrase = getReasonPhrase(copyHeader.status_line.statusCode);
=======
    copyHeader.status_line.reasonPhrase = request.getReasonPhrase();
>>>>>>> 12802e200d6aa2f7c4ddb26f941b49808c3641d2
    response.setHeader(copyHeader);
}
