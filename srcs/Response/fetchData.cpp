#include "../../Includes/Response/response.hpp"
#include "../../Includes/utils.hpp"
#include "../../Includes/Request/HttpRequest.hpp"

void getDataFromRequest(HttpRequest &request, Response &response){
    std::cout << "status code from request: " << (e_StatusCode)request.getStatusCode() << std::endl;
    std::cout << "method: " << request.getMethod() << std::endl;
    response.fetch_data_from_request((e_StatusCode)request.getStatusCode(), request.getMethod());
}
