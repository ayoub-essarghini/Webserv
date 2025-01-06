#include "Server.hpp"

using namespace std;

Server::Server(int port, Config &config) : server_socket(port), server_config(config) {}

string Server::readRequest(int client_sockfd)
{
    char buffer[1024] = {0};
    string request = "";
    int valread = read(client_sockfd, buffer, 1024);
    if (valread < 0)
    {
        cerr << "Error reading from socket" << endl;
        return "";
    }
    request.append(buffer, valread);
    return request;
}

void Server::start()
{
    server_socket.bindSocket();
    server_socket.listenSocket();
    cout << "Server is listening on port " << server_socket.getPort() << "..." << endl;

    // Create epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        return;
    }

    // Register server socket with epoll to monitor incoming connections
    
    ev.events = EPOLLIN;
    ev.data.fd = server_socket.getSocketFd();
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket.getSocketFd(), &ev) == -1)
    {
        perror("epoll_ctl");
        return;
    }

    // Holds events for epoll_wait

    while (true)
    {
        // Wait for events on the server socket and any client sockets
        int num_events = epoll_wait(epoll_fd, events.data(), events.size(), -1); // Blocking call
        if (num_events == -1)
        {
            cerr << "epoll_wait" << endl;
            break;
        }

        for (int i = 0; i < num_events; ++i)
        {
            int current_fd = events[i].data.fd;

            if (current_fd == server_socket.getSocketFd())
            {
                // Server socket has incoming connections
                int client_sockfd = server_socket.acceptConnection();
                if (client_sockfd < 0)
                {
                    if (errno == EWOULDBLOCK || errno == EAGAIN)
                    {
                        continue; // No pending connections
                    }
                    cerr << "Error accepting connection" << endl;
                    continue;
                }

                // Set client socket to non-blocking
                fcntl(client_sockfd, F_SETFL, O_NONBLOCK);

                // Register client socket with epoll to monitor for reading
                ev.events = EPOLLIN | EPOLLET; // Use edge-triggered mode for non-blocking I/O
                ev.data.fd = client_sockfd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_sockfd, &ev) == -1)
                {
                    perror("epoll_ctl");
                    close(client_sockfd);
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                // Client socket is ready to be read from
                std::string request = readRequest(current_fd);

                if (request.empty())
                {

                    cout << "Client has closed the connection" << endl;
                    // Client has closed the connection
                    close(current_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                }
                else
                {
                    requestToParse += request;
                    if (requestToParse.find("\r\n\r\n") != std::string::npos || requestToParse.find("\n\n") != std::string::npos)
                    {
                        std::cout << "Received complete request: " << request << std::endl;
                        handleRequest(current_fd, requestToParse, epoll_fd);
                        requestToParse.clear(); // Clear the buffer for the next request
                        // close(current_fd);
                        // epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    }
                    else
                    {
                        // Not a complete request yet, wait for more data
                        continue;
                    }
                }
            }

            else if (events[i].events & EPOLLOUT)
            {

                cout << "Client socket is ready to be written to" << endl;
                // Get the stored response info for this client
                if (responses_info.find(current_fd) != responses_info.end())
                {
                    Response response;
                    ResponseInfos &response_info = responses_info[current_fd];

                    // Set up response
                    response.setStatus(response_info.status, response_info.statusMessage);
                    response.addHeader("Content-Type", "text/html");
                    if (response_info.status == REDIRECTED)
                    {
                        response.addHeader("Location", request.getDecodedPath() + "/");
                    }
                   
                    response.setBody(response_info.body);

                    string response_str = response.getResponse();
                    ssize_t bytes_written = write(current_fd, response_str.c_str(), response_str.length());

                    if (bytes_written == -1)
                    {
                        // Handle write error, possibly with retry logic
                        cerr << "Error writing to socket" << endl;
                        close(current_fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                    }
                    else if (bytes_written < response_str.length())
                    {
                        // If we haven't written all the data, wait for another EPOLLOUT
                        responses_info[current_fd].body = response_str.substr(bytes_written);
                        return;
                    }

                    // Clean up after writing the full response
                    responses_info.erase(current_fd);
                    close(current_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                }
            }

            else if (events[i].events & EPOLLERR)
            {
                // Handle errors for client sockets
                std::cerr << "Error on socket: " << current_fd << std::endl;
                close(current_fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
            }
        }
    }

    close(epoll_fd);
}

void Server::handleRequest(int client_sockfd, string req, int epoll_fd)
{
    try
    {
        HttpParser parser;
        request = parser.parse(req);

        // Store response info for this client socket
        responses_info[client_sockfd] = processRequest(request);

        // Modify the socket to monitor for write events (EPOLLOUT)
        
        ev.events = EPOLLOUT | EPOLLET; // Edge-triggered output
        ev.data.fd = client_sockfd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_sockfd, &ev) == -1)
        {
            perror("epoll_ctl");
            return;
        }
    }
    catch (int &code)
    {
        // If an error occurs, prepare an error response
        Response response;
        response.setStatus(code, Request::generateStatusMsg(code));
        response.addHeader(to_string(code), Request::generateStatusMsg(code));
        response.addHeader("Content-Type", "text/html");
        response.setBody(Request::generateErrorPage(code));

        // Store response info for this client socket
        responses_info[client_sockfd] = ServerUtils::ressourceToResponse(Request::generateErrorPage(code), code);

        // Modify the socket to monitor for write events (EPOLLOUT)
        
        ev.events = EPOLLOUT | EPOLLET; // Edge-triggered output
        ev.data.fd = client_sockfd;

        if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_sockfd, &ev) == -1)
        {
            perror("epoll_ctl");
            return;
        }
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;
    }
}

ResponseInfos Server::processRequest(const Request &request)
{
    
    if (request.getMethod() == GET)
    {
        return handleGet(request);
    }

    else if (request.getMethod() == POST)
    {

        return handlePost(request);
    }
    else if (request.getMethod() == DELETE)
    {
        return handleDelete(request);
    }
    else
    {

        return ServerUtils::ressourceToResponse(ServerUtils::generateErrorPage(NOT_EXIST), NOT_EXIST);
    }
}

ResponseInfos Server::handleGet(const Request &request)
{


    string url = request.getDecodedPath();
    Location bestMatch;
    if (!matchLocation(bestMatch, url))
    {
        RessourceInfo ressource;
        string f_path = "src/" + server_config.getRoot() + url;
        ressource.autoindex = false;
        ressource.indexFiles = server_config.getIndexFiles();
        ressource.path = f_path;
        ressource.root = server_config.getRoot();
        ressource.url = url;
        // std::cout << "No matching location found" << std::endl;
        // cout << "handle get function 1 \n"<< serveRessourceOrFail(ressource) << endl;
        return serveRessourceOrFail(ressource);
    }

    std::cout << "Best matching location: " << bestMatch << std::endl;
    RessourceInfo ressource;
    // // Construct full filesystem path
    cout << "URL: " << url << endl;
    string fullPath = "src" + bestMatch.root + url;
    ressource.autoindex = true;
    ressource.indexFiles = bestMatch.index_files;
    ressource.path = fullPath;
    ressource.root = bestMatch.root;
    ressource.url = url;

   
    if (ServerUtils::isMethodAllowed(request.getMethod(), bestMatch.allow_methods))
        return serveRessourceOrFail(ressource);
    else
        return ServerUtils::ressourceToResponse(Request::generateErrorPage(NOT_ALLOWED), NOT_ALLOWED);
}

ResponseInfos Server::handlePost(const Request &request)
{
    std::cout << "Handling POST request......\n"
              << request.getBody() << std::endl;

    std::map<std::string, Location> locs = server_config.getLocations();
    std::string url = request.getDecodedPath();

    Location bestMatch;
    size_t bestMatchLength = 0;
    bool found = false;

    // Match location for the upload
    // if (matchLocation(bestMatch, url))
    // {
    //     // Check if the request uses chunked transfer encoding
    //     if (request.hasHeader("Transfer-Encoding") && request.getHeader("Transfer-Encoding") == "chunked")
    //     {
    //         // Path to save the uploaded file
    //         std::string uploadPath = "src" + bestMatch.upload_dir + "/" + ServerUtils::generateUniqueString();
    //         std::cout << "Uploading file to: " << uploadPath << std::endl;

    //         std::ofstream outFile(uploadPath.c_str(), std::ios::binary);
    //         if (!outFile)
    //         {
    //             return ServerUtils::ressourceToResponse(ServerUtils::generateErrorPage(INTERNAL_SERVER_ERROR), INTERNAL_SERVER_ERROR);
    //         }

    //         // Process chunked transfer encoding
    //         std::string chunk;
    //         size_t totalBytesReceived = 0;
    //         while (request.hasMoreData())
    //         {
    //             cout << "Request has more data" << endl;
    //             chunk = request.getNextChunk();
    //             cout << "Chunk: " << chunk.c_str() << endl;
    //             // if (chunk.empty())
    //             // {
    //             //     break; // End of chunks (empty chunk indicates completion in chunked encoding)
    //             // }

    //             // Write the chunk to the file
    //             outFile.write(chunk.c_str(), chunk.size());
    //             totalBytesReceived += chunk.size();

    //             std::cout << "Received chunk of size: " << chunk.size() << " bytes." << std::endl;
    //         }

    //         outFile.close();
    //         std::cout << "Upload complete, total size: " << totalBytesReceived << " bytes." << std::endl;

    //         return ServerUtils::ressourceToResponse("", CREATED);
    //     }
    //     else
    //     {
    //         // Handle non-chunked upload here (same as your current implementation)
    //         std::string uploadPath = "src" + bestMatch.upload_dir + "/" + ServerUtils::generateUniqueString();
    //         std::cout << "Uploading file to: " << uploadPath << std::endl;

    //         std::ofstream outFile(uploadPath.c_str(), std::ios::binary);
    //         if (!outFile)
    //         {
    //             return ServerUtils::ressourceToResponse(ServerUtils::generateErrorPage(INTERNAL_SERVER_ERROR), INTERNAL_SERVER_ERROR);
    //         }
    //         outFile.write(request.getBody().c_str(), request.getBody().size());
    //         outFile.close();

    //         return ServerUtils::ressourceToResponse("", CREATED);
    //     }
    // }

    return ServerUtils::ressourceToResponse(ServerUtils::generateErrorPage(NOT_FOUND), NOT_FOUND);
}

ResponseInfos Server::handleDelete(const Request &request)
{
    map<string, Location> locs = server_config.getLocations();
    string url = request.getDecodedPath();

    Location bestMatch;
    size_t bestMatchLength = 0;
    bool found = false;

    for (const auto &loc : locs)
    {
        const string &locationPath = loc.first;
        if (url.find(locationPath) == 0 && (url == locationPath || url[locationPath.length()] == '/'))
        {
            if (locationPath.length() > bestMatchLength)
            {
                found = true;
                bestMatch = loc.second;
                bestMatchLength = locationPath.length();
            }
        }
    }

    if (found)
    {
        return ServerUtils::ressourceToResponse("<html><body><h1>DELETE Operation Successful on " + bestMatch.root + "</h1></body></html>", CREATED);
    }

    return ServerUtils::ressourceToResponse(ServerUtils::generateErrorPage(NOT_FOUND), NOT_FOUND);
}

bool Server::matchLocation(Location &loc, const string url)
{
    map<string, Location> locs = server_config.getLocations();
    map<string, Location>::const_iterator loc_it = locs.begin();
    Location bestMatch;
    size_t bestMatchLength = 0;
    bool found = false;

    for (; loc_it != locs.end(); loc_it++)
    {
        const string &locationPath = loc_it->first;

        // Check if URL starts with this location path
        if (url.find(locationPath) == 0)
        {

            if (locationPath.length() > bestMatchLength)
            {
                // std::cout << "Found better match: " << locationPath << std::endl;
                found = true;
                bestMatch = loc_it->second;

                bestMatchLength = locationPath.length();
            }
        }
    }
    loc = bestMatch;
    return found;
}

ResponseInfos Server::serveRessourceOrFail(RessourceInfo ressource)
{

    switch (ServerUtils::checkResource(ressource.path))
    {
    case DIRECTORY:
        return ServerUtils::serverRootOrRedirect(ressource);
        break;
    case REGULAR:
        return ServerUtils::serveFile(ressource.path, OK);
        break;
    case NOT_EXIST:
        return ServerUtils::serveFile("src/www/404.html", NOT_FOUND);
        break;
    case UNDEFINED:
        return ServerUtils::serveFile("src/www/404.html", NOT_FOUND);
        break;
    default:
        return ServerUtils::serveFile("src/www/404.html", NOT_FOUND);
        break;
    }
}

ostream &operator<<(ostream &os, const Request &request)
{
    os << "-------------Method:---------\n " << request.getMethod() << endl;
    os << "-------------URI:----------\n"
       << request.getPath() << endl;
    os << "-------------Version:---------\n " << request.getVersion() << endl;
    os << "-------------Headers:----------\n"
       << endl;
    for (const auto &header : request.getHeaders())
    {
        os << header.first << ": " << header.second << endl;
    }
    os << "---------------Body:----------------\n"
       << request.getBody() << endl;
    return os;
}
