#pragma once

#include "../http/Socket.hpp"
#include "../http/Request.hpp"
#include "../http/Response.hpp"
#include <string>
#include <map>

using namespace std;

class Server {
public:
    Server(int port);
    void start();
    
private:
    Socket server_socket;
    void handleRequest(int client_sockfd);
    string processRequest(const Request& request);
};

