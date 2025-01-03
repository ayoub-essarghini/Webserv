#pragma once
#include "../utils/MyType.hpp"
#include "Response.hpp"
#include <string>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <vector>
#include <map>

using namespace std;

class ServerUtils
{
public:
    ServerUtils();
    static string getMimeType(const std::string &filePath);
    static File_Type checkResource(const std::string &fullPath);
    static string serverRootOrRedirect(RessourceInfo ressource);
    static string generateErrorPage(int statusCode);
    static string serveFile(const std::string& filePath);
    static string generateDirectoryListing(const string &dirPath);
    static string handleRedirect(const string &redirectUrl, int statusCode);

};
