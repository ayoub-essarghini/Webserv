#include "Server.hpp"
#include <iostream>

Server::Server(int port) : server_socket(port) {}

void Server::start() {
    server_socket.bindSocket();
    server_socket.listenSocket();
    cout << "Server is listening on port " << server_socket.getPort() << "..." << endl;

    while (true) {
        int client_sockfd = server_socket.acceptConnection();
        handleRequest(client_sockfd);
        close(client_sockfd);
    }
}

void Server::handleRequest(int client_sockfd) {
    char buffer[1024];
    int bytes_read = read(client_sockfd, buffer, sizeof(buffer) - 1);
    if (bytes_read <= 0) return;

    buffer[bytes_read] = '\0';
    Request request(buffer);

    string response_body = processRequest(request);

    Response response;
    response.setStatus(200, "OK");
    response.addHeader("Content-Type", "text/html");
    response.setBody(response_body);

    string response_str = response.getResponse();
    write(client_sockfd, response_str.c_str(), response_str.length());
}

string Server::processRequest(const Request& request) {
    if (request.getMethod() == "GET" && request.getPath() == "/") {
        return "<html><body><h1>Welcome to the server!</h1></body></html>";
    } else {
        return "<html><body><h1>404 Not Found</h1></body></html>";
    }
}
