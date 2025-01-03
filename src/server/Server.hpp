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
    ResponseInfos response_info;
    void handleRequest(int client_sockfd, string req);
    string readRequest(int client_sockfd);
    ResponseInfos processRequest(const Request &request);
    ResponseInfos handleGet(const Request &request);
    ResponseInfos handlePost(const Request &request);
    ResponseInfos handleDelete(const Request &request);
    ResponseInfos serveRessourceOrFail(RessourceInfo ressource);
    bool matchLocation(Location &loc, const string url);

    string requestToParse;
};

ostream &operator<<(ostream &os, const Request &request);
