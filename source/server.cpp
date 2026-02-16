#include "../includes/server.h"

Server::Server(int port){
    WSADATA _wsa_data;
    int wsastart = WSAStartup(MAKEWORD(2,2), &_wsa_data);
    if(wsastart != 0){
        // MAKEWORD requests version 2 of WinSock
        std::cout<<"Error: "<<wsastart;
        std::exit(-1);
    }
    // create socket
    this->_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(_socket == SOCKET_ERROR){
        std::cout<<"Socket Creation Error: \n";
        std::exit(-1);
    }else{
        std::cout<<"Socket ready to go\n";
    }
    _port = port;

    // bind socket
    _socket_address.sin_family = AF_INET;
    _socket_address.sin_port = htons(_port);
    _socket_address.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(_socket, (struct sockaddr*)&_socket_address,sizeof(_socket_address)) == SOCKET_ERROR){
        std::cout<<"Binding error\n";
        std::exit(-1);
    } 
    Handler getHandler;
    getHandler._name = "getHandler";
    getHandler._callBack = [](std::string id){std::cout<<"getHandler called with id: "<<id<<"\n";};

    _router.addRoute({"GET"},"/",getHandler);
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
    if(client_socket == INVALID_SOCKET) return HttpRequest();

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

    HttpRequest request((std::string)(method), (std::string)(path), std::unordered_map<std::string, std::string>(),(std::string)body, client_socket);

    return request;
}

void Server::respondRequest(HttpRequest request){
    if(request.clientSocket == NULL) return;

    _router.route(request); //

    char message[] = "HTTP/1.1 200 OK\nServer: Hello\nContent-Length: 13\nContent-Type: text/plain";
    printf("Responding.....\n");
    printf(message);
    send(request.clientSocket,message,(int)strlen(message),0 );
    closesocket(request.clientSocket);
    printf("\n..Done\n");

}
void Server::serve() {
    startListening();

    std::cout << ">>>> Server is live and listening: Socket => "<<_socket<<" <<<<\n";
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