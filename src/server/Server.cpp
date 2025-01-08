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

    ev.events = EPOLLIN | EPOLLHUP | EPOLLERR; // Use edge-triggered mode for non-blocking I/O
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
                   
                    cerr << "Error accepting connection" << endl;
                    continue;
                }

                // Set client socket to non-blocking
                // fcntl(client_sockfd, F_SETFL, O_NONBLOCK);

                // Register client socket with epoll to monitor for reading
                ev.events = EPOLLIN ; // Use edge-triggered mode for non-blocking I/O
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
                string req = readRequest(current_fd);

                if (req.empty())
                {
                    // Handle read error, possibly with retry logic
                    cerr << "Error reading from socket" << endl;
                    close(current_fd);
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                }
                

                cout << "Request: " << req << endl;
                //   struct epoll_event ev;
                // ev.events = EPOLLIN | EPOLLET;
                // ev.data.fd = current_fd;
                // epoll_ctl(epoll_fd, EPOLL_CTL_MOD, current_fd, &ev);
                // if (req.empty())
                // {
                //     // Handle read error, possibly with retry logic
                //     cerr << "Error reading from socket" << endl;
                //     close(current_fd);
                //     epoll_ctl(epoll_fd, EPOLL_CTL_DEL, current_fd, NULL);
                // }
                // else
                // {

                //     // Handle the request
                    handleRequest(current_fd, req, epoll_fd);
                // }
            }

            else if (events[i].events & EPOLLOUT)
            {

                // cout << "Client socket is ready to be written to" << endl;
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

// void Server::handleRequest(int client_sockfd, string req, int epoll_fd)
// {
//     try
//     {
//         if (chunked_uploads.find(client_sockfd) == chunked_uploads.end())
//         {
//             // New request
//             HttpParser parser;
//             // cout << i << ": " <<  req << endl;
//             request = parser.parse(req);

//                    if (request.getMethod() == "POST" &&
//                 request.hasHeader("Transfer-Encoding") &&
//                 request.getHeader("Transfer-Encoding") == "chunked")
//             {
//                 cout << "Starting chunked upload" << endl;
//                 // Initialize chunked upload state
//                 ChunkedUploadState state;
//                 state.headers_parsed = true;
//                 state.content_remaining = 0;
//                 state.upload_path = "uploads/" + ServerUtils::generateUniqueString();
//                 state.output_file.open(state.upload_path, std::ios::binary);

//                 chunked_uploads[client_sockfd] = state;

//                 // Keep monitoring for more data
//                 struct epoll_event ev;
//                 ev.events = EPOLLIN | EPOLLET;
//                 ev.data.fd = client_sockfd;
//                 epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_sockfd, &ev);
//             }
//             else
//             {

//                 cout << "Processing non-chunked data" << endl;
//                 // Handle non-chunked request as before
//                 responses_info[client_sockfd] = processRequest(request);

//                 struct epoll_event ev;
//                 ev.events = EPOLLOUT | EPOLLET;
//                 ev.data.fd = client_sockfd;
//                 epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_sockfd, &ev);
//             }
//         }
//         else
//         {
//             cout << "Processing chunked data" << endl;
//             // Continue processing chunked upload
//             processChunkedData(client_sockfd, req, epoll_fd);
//         }
//     }
//     catch (int code)
//     {

//         cout << "Error code: " << code << endl;

//         Response response;
//         response.setStatus(code, Request::generateStatusMsg(code));
//         response.addHeader(to_string(code), Request::generateStatusMsg(code));
//         response.addHeader("Content-Type", "text/html");
//         response.setBody(Request::generateErrorPage(code));

//         responses_info[client_sockfd] = ServerUtils::ressourceToResponse(Request::generateErrorPage(code), code);

//         struct epoll_event ev;
//         ev.events = EPOLLOUT | EPOLLET;
//         ev.data.fd = client_sockfd;
//         epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_sockfd, &ev);
//     }
//     catch (exception &e)
//     {
//         cerr << e.what() << endl;
//     }
// }

void Server::processChunkedData(int client_sockfd, const string &data, int epoll_fd)
{
    ChunkedUploadState &state = chunked_uploads[client_sockfd];
    state.partial_request += data;

    while (true)
    {
        // Find chunk size line
        size_t pos = state.partial_request.find("\r\n");
        if (pos == string::npos)
            return; // Need more data

        // Parse chunk size
        string chunk_size_str = state.partial_request.substr(0, pos);
        size_t chunk_size;
        std::stringstream ss;
        ss << std::hex << chunk_size_str;
        ss >> chunk_size;

        // Check if this is the last chunk
        if (chunk_size == 0)
        {
            // Upload complete
            state.output_file.close();
            chunked_uploads.erase(client_sockfd);

            // Send success response
            responses_info[client_sockfd] = ServerUtils::ressourceToResponse("", CREATED);

            struct epoll_event ev;
            ev.events = EPOLLOUT;
            ev.data.fd = client_sockfd;
            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_sockfd, &ev);
            return;
        }

        // Check if we have the full chunk
        if (state.partial_request.length() < pos + 2 + chunk_size + 2)
        {
            return; // Need more data
        }

        // Write chunk to file
        state.output_file.write(
            state.partial_request.data() + pos + 2, // Skip size line and CRLF
            chunk_size);

        // Remove processed chunk from buffer
        state.partial_request = state.partial_request.substr(pos + 2 + chunk_size + 2);
    }
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
    RessourceInfo ressource;
    if (!matchLocation(bestMatch, url))
    {
        string f_path = server_config.getRoot() + url;
        ressource.autoindex = false;
        ressource.indexFiles = server_config.getIndexFiles();
        ressource.path = f_path;
        ressource.root = server_config.getRoot();
        ressource.url = url;
        return serveRessourceOrFail(ressource);
    }

    string fullPath = bestMatch.root + url;
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
    // std::cout << "Handling POST request......\n"
    //   << request.getBody() << std::endl;

    std::map<std::string, Location> locs = server_config.getLocations();
    std::string url = request.getDecodedPath();

    Location bestMatch;
    size_t bestMatchLength = 0;
    bool found = false;

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

    // cout << "serveRessourceOrFail " << ressource.path << endl;

    switch (ServerUtils::checkResource(ressource.path))
    {
    case DIRECTORY:
        return ServerUtils::serverRootOrRedirect(ressource);
        break;
    case REGULAR:
        return ServerUtils::serveFile(ressource.path, OK);
        break;
    case NOT_EXIST:
        return ServerUtils::serveFile("www/404.html", NOT_FOUND);
        break;
    case UNDEFINED:
        return ServerUtils::serveFile("www/404.html", NOT_FOUND);
        break;
    default:
        return ServerUtils::serveFile("www/404.html", NOT_FOUND);
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
