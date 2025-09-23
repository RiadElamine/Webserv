#include "../includes/response.hpp"


int main() {
    HttpRequest request;
    Response response;

    getDataFromRequest(request, response);
    response.execute_method();
    std::cout << response.getResponse();
    return (0);
}