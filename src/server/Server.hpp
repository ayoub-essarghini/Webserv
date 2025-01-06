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
#include <sys/epoll.h>

#include <fcntl.h>
#include <map>
#include <sys/stat.h>
#include <vector>
#include <algorithm>

#define MAX_EVENTS 10
using namespace std;

class Server
{
public:
    Server(int port, Config &config);
    void start();

private:
    Request request;
    Socket server_socket;
    struct epoll_event ev;
    vector<epoll_event> events = vector<epoll_event>(MAX_EVENTS);
    Config server_config;
    map<int, ResponseInfos> responses_info;
    void handleRequest(int client_sockfd, string req, int epoll_fd);
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
