#pragma once

#include "../http/Socket.hpp"
#include "../http/Request.hpp"
#include "../http/HttpParser.hpp"
#include "../http/ServerUtils.hpp"
#include "../http/Response.hpp"
#include "../config/Config.hpp"
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>
#include <fcntl.h>
#include <map>
#include <sys/stat.h>
#include <vector>
#include <algorithm>

using namespace std;



class Server
{
public:
    Server(int port, Config &config);
    void start();

private:
    Socket server_socket;
    Config server_config;
    void handleRequest(int client_sockfd, string req);
    string readRequest(int client_sockfd);
    string processRequest(const Request &request);
    string handleGet(const Request &request);
    string handlePost(const Request &request);
    string handleDelete(const Request &request);
    string serveRessourceOrFail(RessourceInfo ressource);
    bool matchLocation(Location &loc, const string url);

    string requestToParse;
};

ostream &operator<<(ostream &os, const Request &request);
