#include "Server.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
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

     cout << response_body << endl;

    Response response;
    response.setStatus(200, "OK");
    response.addHeader("Content-Type", "text/html");
    response.setBody(response_body);

    string response_str = response.getResponse();
    write(client_sockfd, response_str.c_str(), response_str.length());
}

string Server::processRequest(const Request &request)
{
    
        if (request.getMethod() == "GET")
            return this->handleGet(request);
        else
        {
            return "<html><body><h1>404 Not Found</h1></body></html>";//this.forbidden()
        }

}



string Server::handleGet(const Request &request)
{

  map<string, Location> locs = server_config.getLocations();
  map<string, Location>::const_iterator it = locs.begin();
    string url = request.getDecodedPath();
    
   
    Location  bestMatch;
    size_t bestMatchLength = 0;

    bool found = false;

    // Iterate through the map of locations
    for (; it != locs.end(); it++) {
        const string& locationPath = it->first;

        // Check if the location is a prefix of the URL
        if (url.find(locationPath) == 0 && (url == locationPath || url[locationPath.length()] == '/')) {
            // If this match is longer than the previous best match, update it
            if (locationPath.length() > bestMatchLength) {
                found = true;
                bestMatch = it->second;
                bestMatchLength = locationPath.length();
            }
        }
    }

    if (found) {
        cout << bestMatch << endl;
        return "Found";
    }


    return "No match found.";
}
