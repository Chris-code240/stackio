#include <vector>
#include <string>
#include <iostream>

struct Error{
    int _code;
    std::vector<std::string> _errors = {};

    Error(int code, std::vector<std::string> errors): _code(code), _errors(errors){}

    std::string dump(){
        std::string message = std::to_string(_code) + "[ERROR]\r\n";

        for(auto i  : _errors) message += "\t- " +i +"\r\n";
        return message;
    }

};