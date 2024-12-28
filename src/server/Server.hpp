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
    Server(int port, Config& config);
    void start();

private:
    Socket server_socket;
    Config server_config;
    void handleRequest(int client_sockfd);
    string processRequest(const Request& request);
    string handleGet(const Request& request);
    string handlePost(const Request& request);
    string handleDelete(const Request& request);
    string handleRedirect(const string& redirectUrl, int statusCode);
    string serveFile(const string& filePath);
    string generateDirectoryListing(const string& dirPath);
    string generateErrorPage(int statusCode, const string &statusText);
    string requestToParse;
};
