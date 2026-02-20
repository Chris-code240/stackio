#include "../includes/router.h"

void Router::addRoute(std::vector<std::string> methods, std::string path,Handler handler ){
    Route route(methods, path,handler);
    _routes.push_back(route);
}

void Router::route(HttpRequest request){

    Error error(404,{});
    bool validPath  = false;
    bool allowedMethod = false;
    for(Route route : _routes){
        if(route._path == request._path){
            validPath = true;
        }else{
            validPath = false;
        }

        if (std::find(route._methods.begin(), route._methods.end(), request._method) != route._methods.end()){
            if(validPath){route._handler._callBack(request); return;}
            allowedMethod = true;
        }else{
            allowedMethod = false;
        }

    }

    if(!validPath) error._errors.push_back("Invalid Path");

    if(!allowedMethod)  error._errors.push_back("Invalid Method");

    if(!validPath && !allowedMethod){
        error._code = 400;
        error._errors = {"Invalid Path", "Invalid Method"};
    }
    // printf(error.dump().c_str());

    std::string message = request.prepareMessage(error.dump());
    send(request.clientSocket,message.c_str(),(int)strlen(message.c_str()),0);
    closesocket(request.clientSocket);

}