#pragma once

#include "../http/Socket.hpp"
#include "../http/Request.hpp"
#include "../http/HttpParser.hpp"
#include "../http/Response.hpp"
#include "../config/Config.hpp"
#include <string>
#include <map>

using namespace std;

struct ClientInfo
{
    string fd;
    char *buffer;
};

class Server {
public:
    Server(int port, Config& config);
    void start();

private:
    Socket server_socket;
    Config server_config;
    map<int, ClientInfo> clients;
    void handleRequest(int client_sockfd,string req);
    string readRequest(int client_sockfd);
    string processRequest(const Request& request);
    string handleGet(const Request& request);
    string handlePost(const Request& request);
    string handleDelete(const Request& request);
    string handleRedirect(const string& redirectUrl, int statusCode);
    string serveFile(const string& filePath);
    string generateDirectoryListing(const string& dirPath);
    string generateErrorPage(int statusCode);
    std::string getMimeType(const std::string& filePath);
    std::string checkResource(const std::string& fullPath);
    string requestToParse;
};

ostream &operator<<(ostream &os, const Request &request);
ostream &operator<<(ostream &os, const Response &response);

