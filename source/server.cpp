#include "../includes/server.h"
#include <string>

void Server::generateAuthCredentials(){
    // generate token and ID
    uint32_t id = reinterpret_cast<uint32_t>(&_config);
    auto token = jwt::create()
                    .set_issuer("auth0")
                    .set_type("JWS")
                    .set_payload_claim("ID", jwt::claim(std::string(std::to_string(id))))
                    .set_issued_at(std::chrono::system_clock::now())
                    .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{3600});
    std::string signed_token = token.sign(jwt::algorithm::hs256{_config._authConfig._secret});
    std::ofstream authFile;
    json data = {{"ID", id}, {"token", signed_token}};
    authFile.open("auth.json");
    if(authFile.is_open()){
        authFile <<data.dump(4);
        authFile.close();
    }
    _config._authConfig._ID = id;
}


std::optional<std::string> getHeader(const HttpRequest& request, std::string key) {
    if (request._headers.empty()) return std::nullopt;

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    for (const auto& [k, v] : request._headers) {
        std::string lowerK = k;
        std::transform(lowerK.begin(), lowerK.end(), lowerK.begin(), ::tolower);
        
        if (lowerK == key) return v;
    }

    return std::nullopt; 
}
void Server::verifyRequest(HttpRequest request) {
    if (request._headers.empty()) {
        throw std::runtime_error("Invalid Request: No headers");
    }

    auto contentType = getHeader(request, "Content-Type");
    if (!contentType.has_value() || contentType.value() != "application/json") {
        throw std::runtime_error("Invalid Request Type");
    }

    if (_config._authConfig._enabled) {
        auto authOpt = getHeader(request, "Authorization");
        if (!authOpt.has_value()) {
            throw std::runtime_error("Authorization header missing");
        }

        std::string auth = authOpt.value();
        
        std::string prefix = "bearer ";
        if (auth.size() <= prefix.size() || auth.substr(0, prefix.size()) != prefix) {
            throw std::runtime_error("Invalid Authorization: Token type");
        }

        std::string token = auth.substr(prefix.size());
        
        auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{_config._authConfig._secret})
                .with_issuer("auth0");
        auto decoded = jwt::decode(token);
        verifier.verify(decoded); // This throws if verification fails
    }
}

Server::Server(Configuration config): _config(config){
    _config = config;
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
    _storageEngine._walFilePath = config._storageConfig._wal_file;

    //
    generateAuthCredentials();
    //
    // bind socket
    _socket_address.sin_family = AF_INET;
    _socket_address.sin_port = htons(_port);
    _socket_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(_socket, (struct sockaddr*)&_socket_address,sizeof(_socket_address)) == SOCKET_ERROR){
        std::cout<<"[SERVER_ERROR] Binding error\n";
        std::exit(-1);
    }

    Handler getHandler("getHandler",[this](std::optional<HttpRequest> request){
        if(!request.has_value()) return;
        // take ID from request then grab data from the store

        HttpRequest req = request.value();
        std::cout<<"Request: GET : \n"<<req._body<<"\n";
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

        if(!request.has_value()) { std::cout<<"No req value\n";return;}
        HttpRequest req = request.value();
        if(!req.clientSocket) {std::cout<<"No client\n";return;}
        // for debugging purpose
        std::string message, writeAheadMessage;
        try{
            verifyRequest(req);
            json data = req._body.size() ? json::parse(req._body) : json::parse("{}");
            if(!data.size()) throw std::runtime_error("Empty Data");
            auto it = data.begin();
            _storageEngine.set(it.key(), it.value());
                // lets test the writeToFile()
            writeAheadMessage = _storageEngine.writeAhead(json({{"op","SET"}, {"key", it.key()}, {"value", it.value()}})).dump();
            message = Response(200,json({{"success", true}})).dump();
            send(req.clientSocket,message.c_str(),(int)strlen(message.c_str()), 0);

        }catch(std::exception e){
            message = Response(400, json({{"message", e.what()}})).dump();
            send(req.clientSocket, message.c_str(), (int)strlen(message.c_str()), 0);
        }catch(...){
            message = Response(400, json({{"message","Honestly have no idea LOL"}})).dump();
            send(req.clientSocket, message.c_str(), (int)strlen(message.c_str()), 0);
        }

        closesocket(req.clientSocket);
    }));

    _router.addRoute({"DELETE"}, "/?[A-Za-z0-9]+", Handler("deleteHandler",[this](std::optional<HttpRequest> request){
        if(!request.has_value()) return;

        HttpRequest req = request.value();

        verifyRequest(req);
        std::string message;

        try{
            _storageEngine.pop(req._path.substr(1));
            message = Response(301,json({{"success", true}, {"message", "Data deleted"}})).dump();
            send(req.clientSocket,message.c_str(),(int)strlen(message.c_str()), 0);
        }catch(std::exception e){
            message = Response(400,json({{"success", false}, {"message", e.what()}})).dump();

            send(req.clientSocket,message.c_str(),(int)strlen(message.c_str()), 0);

        }catch(...){
            message = Response(400,json({{"success", false}, {"message", "Unknown Error"}})).dump();

            send(req.clientSocket,message.c_str(),(int)strlen(message.c_str()), 0);
        }
        closesocket(req.clientSocket);
    }));

    _router.addRoute({"POST"}, "/token",Handler("postTokenHandler", [this](std::optional<HttpRequest> request){
        if(!request.has_value()) return;
        if(!_config._authConfig._enabled){
            std::cout<<"Auth not enabled\n";
            closesocket(request.value().clientSocket);
            return;
        }
     
        HttpRequest  req = request.value();
        std::string message;
        try {
            json data = req._body.size() ? json::parse(req._body) : json::parse("{}");
            auto it = data.begin();
            if(!it.key().size() || !it.value().size() || it.key() != "token"){
                json res;
                res["success"] = false;
                res["message"] = "Invalid Request Data";
                std::string message = Response(400,res).dump();
                send(req.clientSocket,message.c_str(), (int)strlen(message.c_str()), 0);
            }else{
                // decode and verify token
                // then send new one upon success

                auto verifier = jwt::verify()
                    .allow_algorithm(jwt::algorithm::hs256{_config._authConfig._secret})
                    .with_issuer("auth0")
                    .leeway(259200);

                auto decoded = jwt::decode(it.value());
                verifier.verify(decoded);
                auto newToken = jwt::create()
                    .set_issuer("auth0")
                    .set_type("JWS")
                    .set_payload_claim("ID", jwt::claim(std::string(std::to_string(_config._authConfig._ID))))
                    .set_issued_at(std::chrono::system_clock::now())
                    .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{3600});
                std::string signed_token = newToken.sign(jwt::algorithm::hs256{_config._authConfig._secret});
                json res = {{"token", signed_token}};
                message = Response(200, res).dump();
  
            }
        }
        catch(const std::exception& e){

            message = Response(400, json({{"message", e.what()}})).dump();
        }
        std::cout<<message<<"";
        send(req.clientSocket,message.c_str(), (int)strlen(message.c_str()), 0);
        closesocket(req.clientSocket);
    }));
}


void Server::routeRequest(HttpRequest request){

    for(auto route : _router._routes){
        if(std::regex_match(request._path, std::regex(route._path)) && std::find(route._methods.begin(), route._methods.end(), request._method) != route._methods.end()){
            route._handler._callBack(request);
            return;
        }
    }
    closesocket(request.clientSocket);


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
    HttpRequest request((std::string)(method), (std::string)(path), parseHeaders(buffer),(std::string)body, client_socket);
    // std::cout<<"\n"<<buffer<<"\n";
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

    std::cout << "\n>>>> Server Is Live <<<<\n"<<"\t[-] Find auth credentials in auth.json\n";
    struct epoll_event events[MAX_EVENTS];

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
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
                if(request.clientSocket)      routeRequest(request);
            }
        }
        
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        if(millis % _config._storageConfig._flush_interval_ms){
            _storageEngine.writeAhead(NULL);
        }

    }

}


Server::~Server(){
    closesocket(_socket);
    WSACleanup();
}