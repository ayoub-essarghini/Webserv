#pragma once
#include "../utils/MyType.hpp"
#include "Response.hpp"
#include "Request.hpp"
#include <string>
#include <iomanip>
#include <iostream>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <vector>
#include <map>

class ServerUtils
{
public:
    ServerUtils();
    static string getMimeType(const std::string &filePath);
    static File_Type checkResource(const std::string &fullPath);
    static ResponseInfos serverRootOrRedirect(RessourceInfo ressource);
    static string generateErrorPage(int statusCode);
    static ResponseInfos ressourceToResponse(string ressource,int statusCode);
    static ResponseInfos serveFile(const std::string& filePath,int code);
    static ResponseInfos generateDirectoryListing(const string &dirPath);
    static string handleRedirect(const string &redirectUrl, int statusCode);

};

ostream &operator<<(ostream &os, const ResponseInfos &response);
