#include <vector>
#include <iostream>
#include <functional>
#include <optional>

struct Handler{
    std::string _name;
    std::function<void(std::string)> _callBack;

    // Handler(std::string name = "handler_name",std::function<void(std::optional<std::string>)> callback) : _name(name), _callBack(callback){}
};