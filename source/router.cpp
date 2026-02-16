#include "../includes/router.h"

void Router::addRoute(std::vector<std::string> methods, std::string path,Handler handler ){
    Route route(methods, path,handler);
    _routes.push_back(route);
}

Response Router::route(HttpRequest request){
    Response res;
    res.success = true;
    res._body = "";
    _routes[0]._handler._callBack(request._body);
    return res;
}