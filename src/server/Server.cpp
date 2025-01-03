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

    // Set server socket to non-blocking
    // fcntl(server_socket.getSocketFd(), F_SETFL, O_NONBLOCK);

    while (true)
    {
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

        string request;
        bool complete = false;
        while (!complete)
        {
            char buffer[1024] = {0};
            int valread = read(client_sockfd, buffer, 1024);

            if (valread < 0)
            {
                if (errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    usleep(1000); // Short sleep to prevent CPU spinning
                    continue;
                }
                cerr << "Error reading from socket" << endl;
                break;
            }
            else if (valread == 0)
            {
                break; // Connection closed
            }

            request.append(buffer, valread);

            // Check for request termination
            if (request.find("\r\n\r\n") != string::npos || request.find("\n\n") != string::npos)
            {
                complete = true;
            }
        }

        if (complete)
        {
            handleRequest(client_sockfd, request);
        }
        close(client_sockfd);
    }
}

void Server::handleRequest(int client_sockfd, string req)
{

    try
    {
        HttpParser parser;
        Request request = parser.parse(req);

        cout << request << endl;

        string response_body = processRequest(request);

        Response response;
        response.setStatus(200, "OK");
        response.addHeader("Content-Type", "text/html");
        response.setBody(response_body);

        string response_str = response.getResponse();
        write(client_sockfd, response_str.c_str(), response_str.length());
    }
    catch (int &code)
    {
        Response response2;
        response2.setStatus(code, Request::generateStatusMsg(code));
        cout << "Test \n"
             << code << Request::generateStatusMsg(code) << endl;
        response2.addHeader(to_string(code), Request::generateStatusMsg(code));
        response2.addHeader("Content-Type", "text/html");
        response2.setBody(Request::generateErrorPage(code));
        string response_res = response2.getResponse();
        cout << "response :\r\n"
             << response_res << endl;
        write(client_sockfd, response_res.c_str(), response_res.length());
    }
}

string Server::processRequest(const Request &request)
{
    if (request.getMethod() == "GET")
    {
        return handleGet(request);
    }
    else if (request.getMethod() == "POST")
    {
        return handlePost(request);
    }
    else if (request.getMethod() == "DELETE")
    {
        return handleDelete(request);
    }
    else
    {
        return ServerUtils::generateErrorPage(405);
    }
}

string Server::handleGet(const Request &request)
{
    string url = request.getDecodedPath();
    Location bestMatch;

    RessourceInfo ressource;
    if (!matchLocation(bestMatch, url))
    {
        string f_path = "src/" + server_config.getRoot() + url;
        ressource.autoindex = false;
        ressource.indexFiles = server_config.getIndexFiles();
        ressource.path = f_path;
        ressource.root = server_config.getRoot();
        ressource.url = url;
        // std::cout << "No matching location found" << std::endl;
        return serveRessourceOrFail(ressource);
    }

    std::cout << "Best matching location: " << bestMatch << std::endl;

    // // Construct full filesystem path
    string fullPath = "src" + bestMatch.root + url;
    ressource.autoindex = false;
    ressource.indexFiles = bestMatch.index_files;
    ressource.path = fullPath;
    ressource.root = bestMatch.root;
    ressource.url = url;
    return serveRessourceOrFail(ressource);

    // cout << "Full Path is : " << fullPath << endl;

    // // Check if path exists
    // struct stat pathStat;
    // if (stat(fullPath.c_str(), &pathStat) != 0)
    // {

    //     std::cout << "Resource not found: " << fullPath << std::endl;
    //     return ServerUtils::serveFile("src/www/404.html");
    // }

    // // Handle directory
    // if (S_ISDIR(pathStat.st_mode))
    // {

    //     // Redirect if no trailing slash
    //     if (url[url.length() - 1] != '/')
    //     {
    //         string redirectUrl = url + "/";
    //         return ServerUtils::handleRedirect(redirectUrl, 301);
    //     }

    //     // Check for index files first
    //     for (const auto &indexFile : bestMatch.index_files)
    //     {
    //         string indexPath = fullPath + "/" + indexFile;
    //         struct stat indexStat;
    //         if (stat(indexPath.c_str(), &indexStat) == 0)
    //         {
    //             return ServerUtils::serveFile(indexPath);
    //         }
    //     }

    //     // If no index file and autoindex is enabled
    //     if (bestMatch.autoindex)
    //     {
    //         return ServerUtils::generateDirectoryListing(fullPath);
    //     }

    //     return ServerUtils::generateErrorPage(403);
    // }
    // // Handle regular file
    // else if (S_ISREG(pathStat.st_mode))
    // {

    //     return ServerUtils::serveFile(fullPath);
    // }

    // return ServerUtils::serveFile("src/www/404.html");
}

string Server::handlePost(const Request &request)
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
        string body = request.getBody();
        return "<html><body><h1>POST Data Received:</h1><p>" + body + "</p></body></html>";
    }

    return ServerUtils::generateErrorPage(404);
}

string Server::handleDelete(const Request &request)
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
        return "<html><body><h1>DELETE Operation Successful on " + bestMatch.root + "</h1></body></html>";
    }

    return ServerUtils::generateErrorPage(404);
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

string Server::serveRessourceOrFail(RessourceInfo ressource)
{

    switch (ServerUtils::checkResource(ressource.path))
    {
    case DIRECTORY:
        return ServerUtils::serverRootOrRedirect(ressource);
        break;
    case REGULAR:
        return ServerUtils::serveFile(ressource.path);
        break;
    case NOT_EXIST:
        return ServerUtils::serveFile("src/www/404.html");
        break;
    case UNDEFINED:
        return ServerUtils::serveFile("src/www/404.html");
        break;
    default:
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
