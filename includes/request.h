#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")


struct HttpRequest{
    std::string _method;
    std::string _path;
    std::unordered_map<std::string, std::string> _headers;
    std::string _body;
    SOCKET clientSocket = NULL;
    HttpRequest(): _method(NULL),_path(NULL), _headers(std::unordered_map<std::string, std::string>()), _body(NULL){}
    HttpRequest(std::string method, std::string path, std::unordered_map<std::string, std::string> headers, std::string body, SOCKET socket):
    _method (method), _path(path), _headers(headers), _body(body) , clientSocket(socket) {}
};


struct Response{
    bool success = false;
    std::string _body;
    std::vector<std::string> error;
};