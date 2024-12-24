#pragma once

#include "../http/Socket.hpp"
#include "../http/Request.hpp"
#include "../http/Response.hpp"
#include "../config/Config.hpp"
#include <string>
#include <map>

using namespace std;

class Server {
public:
    Server(int port,Config& config);
    void start();
    
private:
    Socket server_socket;
    Config server_config;
    void handleRequest(int client_sockfd);
    string processRequest(const Request& request);
    string handleGet(const Request& request);
    string notFound();
    string forbidden();
    string requestToParse;
};

