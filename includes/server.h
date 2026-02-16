
#include "storage_engine.h"
#include "router.h"

class Server{
    private:
        SOCKET _socket;
        struct sockaddr_in _socket_address;
        int _port;
        std::string _ip_address;
        StorageEngine _storageEngine;
        Router _router;
    public:
        Server(int port);
        SOCKET acceptRequest();
        bool startListening();

        HttpRequest createRequest();
        void routeRequest(HttpRequest  request);
        void respondRequest(HttpRequest  request);
        void serve();

        ~Server();
        // _storageEngine.

};


