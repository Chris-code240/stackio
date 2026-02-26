
#include "storage_engine.h"
#include "router.h"
#include "config.h"
#include "wepoll.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#define MAX_EVENTS 10
class Server{
    private:
        SOCKET _socket;
        struct sockaddr_in _socket_address;
        int _port;
        std::string _ip_address;
        StorageEngine _storageEngine;
        Router _router;
        Configuration _config;
    public:
        Server(Configuration config);
        SOCKET acceptRequest();
        bool startListening();

        HttpRequest createRequest(std::optional<int> clientSocket);
        Response createResponse(HttpRequest request, int statusCode = 200);
        void routeRequest(HttpRequest  request);
        void respondRequest(HttpRequest  request);
        void serve();
        void replayWAL();
        void generateAuthCredentials();
        void verifyRequest(HttpRequest request);
        ~Server();
        // _storageEngine.

};


