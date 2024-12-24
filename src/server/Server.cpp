#include "Server.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

Server::Server(int port, Config &config) : server_socket(port), server_config(config) {}

void Server::start()
{
    server_socket.bindSocket();
    server_socket.listenSocket();
    cout << "Server is listening on port " << server_socket.getPort() << "..." << endl;

    while (true)
    {
        int client_sockfd = server_socket.acceptConnection();
        handleRequest(client_sockfd);
        close(client_sockfd);
    }
}

void Server::handleRequest(int client_sockfd)
{
    char buffer[1024];
    int bytes_read = read(client_sockfd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0)
        return;

    buffer[bytes_read] = '\0';
    
    // cout << buffer << endl;
    Request request(buffer);
    string line  = buffer;
    string response_body;

     response_body = processRequest(request);

    Response response;
    response.setStatus(200, "OK");
    response.addHeader("Content-Type", "text/html");
    response.setBody(response_body);

    string response_str = response.getResponse();
    write(client_sockfd, response_str.c_str(), response_str.length());
}

string Server::processRequest(const Request &request)
{
    
        if (request.getMethod() == "GET" && !request.getPath().empty())
            return this->handleGet(request);
        else
        {
            return "<html><body><h1>404 Not Found</h1></body></html>";//this.forbidden()
        }

}

string Server::handleGet(const Request &request)
{
    Location pathInfos = server_config.getLocation(request.getPath());

    if (!pathInfos.index_files.empty())
    {
        vector<string>::const_iterator it = pathInfos.index_files.begin();

        string filename = "src/";
        filename.append(pathInfos.root);
        filename.append(*it);
        ifstream index(filename);
        if (!index)
        {
            throw runtime_error("Cannot open file");
        }
        string line;
        string response;
        while (getline(index, line))
        {
            response.append(line);
        }
        return response;
    }

    return "404";
}
