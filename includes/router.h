#include "handler.h"

struct Route{
    std::vector<std::string> _methods; 
    std::string _path;
    Handler _handler;

    Route(std::vector<std::string> methods, std::string path,Handler handler): _methods(methods), _path(path), _handler(handler){}
};
class Router{
    public:
        std::vector<Route > _routes;

        void addRoute(std::vector<std::string> methods, std::string path,Handler handler );

        Response route(HttpRequest request);   
};


