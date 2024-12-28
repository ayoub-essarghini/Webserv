#include "Server.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <sys/stat.h>
#include <dirent.h>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <algorithm>

using namespace std;

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

    buffer[bytes_read] = '\0'; // Null-terminate the buffer

    // Parse the incoming request
    Request request(buffer);
    string response_body = processRequest(request);

    // Create and send response
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
        return generateErrorPage(405, "Method Not Allowed");
    }
}

string Server::handleGet(const Request &request)
{
    map<string, Location> locs = server_config.getLocations();
    string url = request.getDecodedPath();

    Location bestMatch;
    size_t bestMatchLength = 0;
    bool found = false;

    // Find the best matching location
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

    if (!found)
    {
        return generateErrorPage(404, "Not Found");
    }

    string path = url.substr(bestMatchLength);
    string fullPath = bestMatch.root + path;

    // Check if redirect is specified
    if (!bestMatch.redirect.empty())
    {
        return handleRedirect(bestMatch.redirect, bestMatch.redirectCode);
    }

    struct stat pathStat;
    if (stat(fullPath.c_str(), &pathStat) != 0) // File or directory does not exist
    {
        return generateErrorPage(404, "Not Found");
    }

    if (S_ISDIR(pathStat.st_mode)) // If it's a directory
    {
        if (bestMatch.autoindex) // If auto-index is enabled
        {
            return generateDirectoryListing(fullPath); // Generate directory listing
        }
        else
        {
            // Check if index file exists in the directory
            for (const auto &indexFile : bestMatch.index_files)
            {
                string indexPath = fullPath + "/" + indexFile;
                if (stat(indexPath.c_str(), &pathStat) == 0) // Index file found
                {
                    return serveFile(indexPath);
                }
            }

            return generateErrorPage(403, "Forbidden");
        }
    }
    else if (S_ISREG(pathStat.st_mode)) // If it's a file
    {
        return serveFile(fullPath);
    }
    else
    {
        return generateErrorPage(404, "Not Found");
    }
}

string Server::handleRedirect(const string &redirectUrl, int statusCode)
{
    stringstream redirectResponse;
    redirectResponse << "<html><body><h1>" << statusCode << " Redirect</h1><p>Redirecting to: <a href=\"" << redirectUrl << "\">" << redirectUrl << "</a></p></body></html>";

    // Respond with redirect header
    Response redirectResponseObj;
    redirectResponseObj.setStatus(statusCode, "Redirect");
    redirectResponseObj.addHeader("Location", redirectUrl);
    redirectResponseObj.setBody(redirectResponse.str());

    return redirectResponseObj.getResponse();
}

string Server::serveFile(const string &filePath)
{
    ifstream file(filePath, ios::in | ios::binary);
    if (!file.is_open())
    {
        return generateErrorPage(404, "Not Found");
    }

    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

string Server::generateDirectoryListing(const string &dirPath)
{
    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
    {
        return generateErrorPage(403, "Forbidden");
    }

    struct dirent *entry;
    stringstream dirContent;

    dirContent << "<html><body><h1>Directory Listing for " << dirPath << "</h1><ul>";

    while ((entry = readdir(dir)) != nullptr)
    {
        // Skip . and .. directories
        if (string(entry->d_name) == "." || string(entry->d_name) == "..")
            continue;

        dirContent << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>";
    }

    dirContent << "</ul></body></html>";
    closedir(dir);
    return dirContent.str();
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

    return generateErrorPage(404, "Not Found");
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

    return generateErrorPage(404, "Not Found");
}

string Server::generateErrorPage(int statusCode, const string &statusText)
{
    stringstream errorPage;
    errorPage << "<html><body><h1>" << statusCode << " " << statusText << "</h1></body></html>";
    return errorPage.str();
}
