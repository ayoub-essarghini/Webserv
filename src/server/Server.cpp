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
        return generateErrorPage(405);
    }
}

string Server::handleGet(const Request &request)
{
    map<string, Location> locs = server_config.getLocations();
    string url = request.getDecodedPath();

    // Debug URL
    std::cout << "Requested URL: " << url << std::endl;

    // First find the best matching location block
    Location bestMatch;
    size_t bestMatchLength = 0;
    bool found = false;

    for (const auto &loc : locs)
    {
        const string &locationPath = loc.first;

        // Check if URL starts with this location path
        if (url.find(locationPath) == 0)
        {

            if (locationPath.length() > bestMatchLength)
            {
                // std::cout << "Found better match: " << locationPath << std::endl;
                found = true;
                bestMatch = loc.second;

                bestMatchLength = locationPath.length();
            }
            // }
        }
    }

    if (!found)
    {
        std::cout << "No matching location found" << std::endl;
        struct stat f_pathStat;
        string f_path = "src/" + server_config.getRoot() + url;
        if (stat(f_path.c_str(), &f_pathStat) != 0)
        {

            std::cout << "Resource not found: " << f_path << std::endl;
            return generateErrorPage(404);
        }
        if (S_ISDIR(f_pathStat.st_mode))
        {

            if (url[url.length() - 1] != '/')
            {
                string redirectUrl = url + "/";
                return handleRedirect(redirectUrl, 301);
            }
            for (const auto &indexFile : server_config.getIndexFiles())
            {
                string indexPath = "src/" + server_config.getRoot() + "/" + indexFile;
                cout << "--------here-------\n";
                cout << indexPath << "\n";
                cout << "---------------\n";
                struct stat indexStat;
                if (stat(indexPath.c_str(), &indexStat) == 0)
                {
                    return serveFile(indexPath);
                }
            }
        }
        if (S_ISREG(f_pathStat.st_mode))
        {
            return serveFile(f_path);
        }

        return generateErrorPage(404);
    }

    std::cout << "Best matching location: " << bestMatch << std::endl;

    // // Construct full filesystem path
    string fullPath = "src" + bestMatch.root + url;
    cout << "Full Path is : " << fullPath << endl;

    // Check if path exists
    struct stat pathStat;
    if (stat(fullPath.c_str(), &pathStat) != 0)
    {

        std::cout << "Resource not found: " << fullPath << std::endl;
        return generateErrorPage(404);
    }

    // Handle directory
    if (S_ISDIR(pathStat.st_mode))
    {
        cout << "------------------IS DIR ------------------------\n";
        // Redirect if no trailing slash
        if (url[url.length() - 1] != '/')
        {
            string redirectUrl = url + "/";
            return handleRedirect(redirectUrl, 301);
        }

        // Check for index files first
        for (const auto &indexFile : bestMatch.index_files)
        {
            string indexPath = fullPath + "/" + indexFile;
            struct stat indexStat;
            if (stat(indexPath.c_str(), &indexStat) == 0)
            {
                return serveFile(indexPath);
            }
        }

        // If no index file and autoindex is enabled
        if (bestMatch.autoindex)
        {
            return generateDirectoryListing(fullPath);
        }

        return generateErrorPage(403);
    }
    // Handle regular file
    else if (S_ISREG(pathStat.st_mode))
    {

        return serveFile(fullPath);
    }

    return generateErrorPage(404);
}

std::string Server::checkResource(const std::string &fullPath)
{
    struct stat pathStat;
    if (stat(fullPath.c_str(), &pathStat) != 0)
    {
        // Path does not exist
        std::cout << "Resource not found: " << fullPath << std::endl;
        return generateErrorPage(404);
    }
    else
    {
        // Path exists, check if it's a directory or a file
        if (S_ISDIR(pathStat.st_mode))
        {
            std::cout << "Resource is a directory: " << fullPath << std::endl;
        }
        else if (S_ISREG(pathStat.st_mode))
        {
            std::cout << "Resource is a regular file: " << fullPath << std::endl;
        }
        else
        {
            std::cout << "Resource is neither a file nor a directory: " << fullPath << std::endl;
        }
        return "Resource found";
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
        return generateErrorPage(404);
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
        return generateErrorPage(403);
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

    return generateErrorPage(404);
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

    return generateErrorPage(404);
}

std::string Server::getMimeType(const std::string &filePath)
{
    std::map<std::string, std::string> mimeTypes = {
        {".html", "text/html"},
        {".htm", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".png", "image/png"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".gif", "image/gif"},
        {".svg", "image/svg+xml"},
        {".ico", "image/x-icon"},
        {".mp3", "audio/mpeg"},
        {".wav", "audio/wav"},
        {".ogg", "audio/ogg"},
        {".mp4", "video/mp4"},
        {".webm", "video/webm"},
        {".txt", "text/plain"},
        {".json", "application/json"},
        {".xml", "application/xml"},
        {".pdf", "application/pdf"},
        {".zip", "application/zip"},
    };

    size_t extPos = filePath.find_last_of('.');
    if (extPos != std::string::npos)
    {
        std::string extension = filePath.substr(extPos);
        if (mimeTypes.find(extension) != mimeTypes.end())
        {
            return mimeTypes[extension];
        }
    }
    return "application/octet-stream"; // Default MIME type for binary files
}

string Server::generateErrorPage(int statusCode)
{

    stringstream errorPage;
    errorPage << "<html><body><h1>" << statusCode << " " << "</h1></body></html>";
    return errorPage.str();
}
