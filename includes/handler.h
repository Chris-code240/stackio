#include <vector>
#include <iostream>
#include <functional>
#include <optional>
#include <string>
#include <optional>
#include "request.h"

struct Handler{
    std::string _name;
    std::function<void(std::optional<HttpRequest>)> _callBack = [this](std::optional<HttpRequest>){
        std::cout<<"@TODO: Implement callback for "<<_name<<" handler";
    };
    Handler(): _name("unknown"){} //for debugging purpose
    Handler(std::string name,std::function<void(std::optional<HttpRequest>)> callback) : _name(name), _callBack(callback){}
};

