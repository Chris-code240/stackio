#include "../includes/server.h"
#include <string>


std::unordered_map<std::string, std::string> parseHeaders(const std::string& request) {
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
Server::Server(int port){
    WSADATA _wsa_data;
    int wsastart = WSAStartup(MAKEWORD(2,2), &_wsa_data);
    if(wsastart != 0){
        // MAKEWORD requests version 2 of WinSock
        std::cout<<"[SERVER_ERROR] WSAStartup"<<wsastart;
        std::exit(-1);
    }
    // create socket
    this->_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == SOCKET_ERROR){
        std::cout<<"[SERVER_ERROR] Socket Creation Error\n";
        std::exit(-1);
    }
    _port = port;

    // bind socket
    _socket_address.sin_family = AF_INET;
    _socket_address.sin_port = htons(_port);
    _socket_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(_socket, (struct sockaddr*)&_socket_address,sizeof(_socket_address)) == SOCKET_ERROR){
        std::cout<<"[SERVER_ERROR] Binding error\n";
        std::exit(-1);
    } 
    Handler getHandler("getHandler",[this](std::optional<HttpRequest> request){
        if(request.has_value() && request.value().clientSocket){
            // take ID from request then grab data from the store
            HttpRequest req = request.value();
            
            std::optional<std::string> pulledData = _storageEngine.get(req._body.substr(1));
            // send data to client
            std::string message;
            if(pulledData.has_value()) {
                message = req.prepareMessage(pulledData.value());
                send(request.value().clientSocket,message.c_str(),(int)strlen(message.c_str()),0);
            }else{
                message = req.prepareMessage("{}");
                send(req.clientSocket,message.c_str(), (int)strlen(message.c_str()), 0);
            }
            closesocket(request.value().clientSocket);
        }
        
    });
    _router.addRoute({"GET"},"/?[A-Za-z0-9]+",getHandler);

    _router.addRoute({"POST"}, "/",Handler("postHandler",[this](std::optional<HttpRequest> request){

        if(!request.has_value()) return;
        HttpRequest req = request.value();
        if(!req.clientSocket) return;
        // for debugging purpose
        try{
            json data = req._body.size() ? json::parse(req._body) : json::parse("{}");
            auto it = data.begin();
            _storageEngine.set(it.key(), it.value());
            json res;
            res["success"] = true;
            std::string message = req.prepareMessage(res.dump());
            send(req.clientSocket,message.c_str(),(int)strlen(message.c_str()), 0);
            std::cout<<"Called PostHandler..."<<message<<"\n";

        }catch(std::exception e){
            std::cout<<"Error: "<<e.what()<<"\n";
        }

        closesocket(req.clientSocket);
    }));
}


void Server::routeRequest(HttpRequest request){

    for(auto route : _router._routes){
        std::cout<<std::regex_match(request._path, std::regex(route._path))<<" - " <<route._path<<" - ";
        for(auto m : route._methods) std::cout<<m<<" ";
        std::cout<<"\n";
        if(std::regex_match(request._path, std::regex(route._path)) && std::find(route._methods.begin(), route._methods.end(), request._method) != route._methods.end()){
            std::cout<<"Handling request..\n";
            route._handler._callBack(request);
            return;
        }
        closesocket(request.clientSocket);
    }

}

SOCKET Server::acceptRequest(){
    int addrLen = sizeof(_socket_address); 
    SOCKET client_socket = accept(_socket, (sockaddr*)&_socket_address, &addrLen);
    return client_socket;
}

bool Server::startListening(){
    if (listen(_socket, 20) == SOCKET_ERROR) {
        std::exit(-1);
        return false;
    }
    return true;
}

HttpRequest Server::createRequest(){
    SOCKET client_socket = acceptRequest();
    if(client_socket == INVALID_SOCKET) {
        std::cout<<"Invalid client socket\n";
        return HttpRequest();
    }

    const int bufferSize = 20000;
    char buffer[bufferSize]{0};
    int bytesReceived = recv(client_socket, buffer, bufferSize - 1, 0);
    if(!bytesReceived) return HttpRequest();
    // lets read request properties [method, path, body,...hederas like content-type later]
    char method[10], path[255], protocol[20];
    sscanf(buffer, "%s %s %s", method, path, protocol);

    char *body = strstr(buffer, "\r\n\r\n");
    if (body) {
        body += 4; // Move pointer past the \r\n\r\n
    }

    HttpRequest request((std::string)(method), (std::string)(path), parseHeaders(buffer),(std::string)body, client_socket);
    
    return request;
}

void Server::respondRequest(HttpRequest request){
    if(request.clientSocket == NULL) return;
    routeRequest(request);
}
void Server::serve() {
    startListening();

    std::cout << ">>>> Server Is Live <<<<\n";
    while (true) {
 
        const HttpRequest request = createRequest();
        if(request.clientSocket) {
            respondRequest(request);
        }
    }
}

Server::~Server(){
    closesocket(_socket);
    WSACleanup();
}