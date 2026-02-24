#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "json/json.hpp"
#include <algorithm>
#include "errors.h"
#include <unordered_map>
#include <sstream>
#include "jwt-cpp/jwt.h"
#pragma comment(lib, "Ws2_32.lib")

using json = nlohmann::json;

struct HttpRequest{
    std::string _method;
    std::string _path;
    std::unordered_map<std::string, std::string> _headers;
    std::string _body;
    SOCKET clientSocket = NULL;
    HttpRequest(): _method(NULL),_path(NULL), _headers(std::unordered_map<std::string, std::string>()), _body(""){}
    HttpRequest(std::string method, std::string path, std::unordered_map<std::string, std::string> headers, std::string body, SOCKET socket):
    _method (method), _path(path), _headers(headers), _body(body) , clientSocket(socket) {}

    std::string prepareMessage(std::string body, std::unordered_map<std::string, std::string> headers = {{"Content-Type", "application/json"}}){
        std::string message = "HTTP/1.1 200 OK\r\n";
        headers["Content-Length"] = std::to_string(body.length());
        for(auto [key, val] : headers){
            message += (key + ":" + val +"\r\n");
        }
        message += "\r\n" + body;

        return message;
        /*
    char message[] = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 17\r\n\r\nhello from server";
    send(request.clientSocket,message,(int)strlen(message),0 );
    closesocket(request.clientSocket);
*/
    }
};



struct Response{
    bool success = false;
    std::string _body;
    std::vector<std::string> error;
    int statusCode = 200;
    bool isJsonResponse = true;
    json _data;
    Response(int code, json data = json::object(),bool isJsonRes = true): statusCode(code), _data(data), isJsonResponse(isJsonRes){

    }
    std::string dump(){
        _body = "HTTP/1.1 " + std::to_string(statusCode) +" " + Errors.at(statusCode) + "\r\n";
        if(isJsonResponse) _body += ("Content-Type: application/json\r\n");
        else  _body += ("Content-Type: text/plain\r\n");
        _body += "Content-Length: " + std::to_string(_data.dump().size()) + "\r\n";

        if(_data.dump().size()){
            _body += "\r\n" + _data.dump();
        }
        return _body;
    }
};


inline std::unordered_map<std::string, std::string> parseHeaders(const std::string& request) {
    std::unordered_map<std::string, std::string> headers;
    std::stringstream stream(request);
    std::string line;

    if (!std::getline(stream, line)) return headers;

    while (std::getline(stream, line) && line != "\r") {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            
            // Extract the Value (skip the space after the colon)
            std::string value = line.substr(colonPos + 1);

            // Trim whitespace/carriage returns
            if (!value.empty() && value.back() == '\r') value.pop_back();
            if (!value.empty() && value.front() == ' ') value.erase(0, 1);

            headers[key] = value;
        }
    }

    return headers;
}


