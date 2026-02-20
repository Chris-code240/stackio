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

inline const std::unordered_map<int, std::string> Errors = {
    {200,"OK"},
    {201, "Created"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Server Error"},
    {503, "Service Unavailable"}
};