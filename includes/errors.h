#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

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

struct status {
    // 2xx Success
    static inline const std::string HTTP_200_OK          = "OK";
    static inline const std::string HTTP_201_CREATED     = "Created";

    // 4xx Client Errors
    static inline const std::string HTTP_400_BAD_REQUEST = "Bad Request";
    static inline const std::string HTTP_401_UNAUTH      = "Unauthorized";
    static inline const std::string HTTP_403_FORBIDDEN   = "Forbidden";
    static inline const std::string HTTP_404_NOT_FOUND   = "Not Found";

    // 5xx Server Errors
    static inline const std::string HTTP_500_INTERNAL    = "Internal Server Error";
    static inline const std::string HTTP_503_SERVICE     = "Service Unavailable";
};

#include <unordered_map>
#include <string>

inline const std::unordered_map<int, std::string> Errors = {
    // 2xx Success
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {204, "No Content"},

    // 3xx Redirection
    {301, "Moved Permanently"},
    {302, "Found"},
    {304, "Not Modified"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},

    // 4xx Client Errors
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {413, "Payload Too Large"},
    {415, "Unsupported Media Type"},
    {422, "Unprocessable Entity"},
    {429, "Too Many Requests"},

    // 5xx Server Errors
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"}
};