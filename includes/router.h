#include "handler.h"
#include "request.h"

struct Route{
    std::vector<std::string> _methods; 
    std::string _path;
    Handler _handler;

    Route(std::vector<std::string> methods, std::string path,Handler handler): _methods(methods), _path(path), _handler(handler){}
};
class Router{
    private:
        std::vector<Route > _routes;
    public:
        void addRoute(std::vector<std::string> methods, std::string path,Handler handler );

        Response route(HttpRequest request);   
};


