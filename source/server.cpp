#include "../includes/server.h"
#include <string>


Server::Server(Configuration config){
    WSADATA _wsa_data;
    int wsastart = WSAStartup(MAKEWORD(2,2), &_wsa_data);
    if(wsastart != 0){
        // MAKEWORD requests version 2 of WinSock
        std::cout<<"[SERVER_ERROR] WSAStartup"<<wsastart;
        std::exit(-1);
    }
    // create socket
    _socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == SOCKET_ERROR){
        std::cout<<"[SERVER_ERROR] Socket Creation Error\n";
        std::exit(-1);
    }
    _port = config._serverConfig._port;

    // bind socket
    _socket_address.sin_family = AF_INET;
    _socket_address.sin_port = htons(_port);
    _socket_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(_socket, (struct sockaddr*)&_socket_address,sizeof(_socket_address)) == SOCKET_ERROR){
        std::cout<<"[SERVER_ERROR] Binding error\n";
        std::exit(-1);
    }
    std::cout<<"Binding..\n"; 
    Handler getHandler("getHandler",[this](std::optional<HttpRequest> request){
        if(!request.has_value()) return;
        // take ID from request then grab data from the store

        HttpRequest req = request.value();
        std::optional<std::string> pulledData = _storageEngine.get(req._path.substr(1));
        // send data to client
        std::string message;
        if(pulledData.has_value()) {
            message = Response(200,json(pulledData)).dump();
            send(request.value().clientSocket,message.c_str(),(int)strlen(message.c_str()),0);
        }else{
            message = Response(400,json("")).dump();
            send(req.clientSocket,message.c_str(), (int)strlen(message.c_str()), 0);
        }
        closesocket(request.value().clientSocket);
        
        return;
        
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

        }catch(std::exception e){
            std::cout<<"Error: "<<e.what()<<"\n";
        }

        closesocket(req.clientSocket);
    }));

    _router.addRoute({"POST"}, "/token",Handler("postTokenHandler", [this](std::optional<HttpRequest> request){
        if(!request.has_value()) return;
     
        HttpRequest  req = request.value();
        std::cout<<"\nClient is: "<<req.clientSocket<<"\n";
        try {
            json data = req._body.size() ? json::parse(req._body) : json::parse("{}");
            auto it = data.begin();
            if(!it.key().size() || !it.value().size() || it.key() != "token"){
                json res;
                res["success"] = false;
                res["message"] = "Invalid Request Data";
                std::string message = Response(400,res).dump();
                send(req.clientSocket, message.c_str(), (int)strlen(message.c_str()), 0);
            }else{
            // auto decoded = jwt::decode(it.value());

            // verify the token that it is us who created it
            // then send new one

                auto newToken = jwt::create()
                    .set_issuer("auth0")
                    .set_type("JWS")
                    .set_payload_claim("ID", jwt::claim(std::string("some_valid_id")))
                    .set_issued_at(std::chrono::system_clock::now())
                    .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{3600});
                std::string signed_token = newToken.sign(jwt::algorithm::hs256{"secret_key_here"});
                json res = {{"token", signed_token}};
                std::cout<<"Token: "<<signed_token<<"\n";
                std::string message = Response(200, res).dump();

                if(!send(req.clientSocket,message.c_str(), (int)strlen(message.c_str()), 0)){
                    std::cout<<"\nMessage not sent\n";
                }else{
                    std::cout<<"\nMessage sent to socket: "<<req.clientSocket<<"\nMessage: \n"<<message<<"\n";
                }
        }
        }
        catch(const std::exception& e)
        {
            std::cerr <<"\nError: "<< e.what() << '\n';
        }
        
        closesocket(req.clientSocket);
    }));
}


void Server::routeRequest(HttpRequest request){

    for(auto route : _router._routes){
        std::cout<<std::regex_match(request._path, std::regex(route._path))<<" - " <<route._path<<" - ";
        if(std::regex_match(request._path, std::regex(route._path)) && std::find(route._methods.begin(), route._methods.end(), request._method) != route._methods.end()){
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
    if (listen(_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::exit(-1);
        return false;
    }
    return true;
}

HttpRequest Server::createRequest(std::optional<int> clientSocket){
    SOCKET client_socket = clientSocket.has_value() ? clientSocket.value() : acceptRequest();
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
    std::cout<<"Request:\n"<<buffer<<"\n";
    HttpRequest request((std::string)(method), (std::string)(path), parseHeaders(buffer),(std::string)body, client_socket);
    
    return request;
}

void Server::respondRequest(HttpRequest request){
    if(request.clientSocket == NULL) return;
    routeRequest(request);
}
void Server::serve() {
    startListening();
    HANDLE epoll_fd = epoll_create1(0);
    if(!(int)(epoll_fd) == -1){
        std::perror("[SERVER_ERROR] epoll");
        std::exit(-1);
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = _socket;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socket, &event) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }

    std::cout << ">>>> Server Is Live <<<<\n";

    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int num_fds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

        for (int n = 0; n < num_fds; ++n) {

            if (events[n].data.fd == _socket) {


                socklen_t addrlen = sizeof(_socket_address);
                int conn_sock = acceptRequest();
                
                event.events = EPOLLIN;
                event.data.fd = conn_sock;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_sock, &event);

            } else {

                HttpRequest request =  createRequest(events[n].data.fd);
                std::cout<<"\nClient is: "<<events[n].data.fd<<"\n";

                if(request.clientSocket)      routeRequest(request);
            }
        }
    }

}


Server::~Server(){
    closesocket(_socket);
    WSACleanup();
}