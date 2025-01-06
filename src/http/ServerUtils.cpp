#include "ServerUtils.hpp"

ServerUtils::ServerUtils() {}

File_Type ServerUtils::checkResource(const std::string &fullPath)
{
    struct stat pathStat;
    if (stat(fullPath.c_str(), &pathStat) != 0)
        return NOT_EXIST;
    else
    {
        if (S_ISDIR(pathStat.st_mode))
            return DIRECTORY;
        else if (S_ISREG(pathStat.st_mode))
            return REGULAR;
        else
            return UNDEFINED;
    }
}

ResponseInfos ServerUtils::serveFile(const string &filePath, int code)
{
    ifstream file(filePath, ios::in | ios::binary);
    if (!file.is_open())
        return ServerUtils::ressourceToResponse(generateErrorPage(NOT_FOUND), NOT_FOUND);

    stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    // cout << "BUFFER: \n" << buffer.str() << endl;
    return ServerUtils::ressourceToResponse(buffer.str(), code);
}

ResponseInfos ServerUtils::serverRootOrRedirect(RessourceInfo ressource)
{
    if (ressource.url[ressource.url.length() - 1] != '/' && ressource.url != "/")
    {
        string redirectUrl = ressource.url + "/";
        return ressourceToResponse(handleRedirect(redirectUrl, REDIRECTED), REDIRECTED);
    }
    vector<string>::const_iterator inedxIter = ressource.indexFiles.begin();
    for (; inedxIter != ressource.indexFiles.end(); inedxIter++)
    {
        string indexPath;

        cout << "is indexed: " << ressource.autoindex << endl;
        if (ressource.autoindex)
            indexPath = "src" + ressource.root + "/" + ressource.url + '/' + *inedxIter;
        else
            indexPath = "src/" + ressource.root + "/" + *inedxIter;
        cout << "--------here-------\n";
        cout << indexPath << "\n";
        cout << "---------------\n";
        struct stat indexStat;
        if (stat(indexPath.c_str(), &indexStat) == 0)
        {
            return ServerUtils::serveFile(indexPath, OK);
        }
    }
    if (ressource.autoindex)
        return ServerUtils::generateDirectoryListing("src/"+ressource.root+ressource.url);
    return ServerUtils::serveFile("src/www/404.html", NOT_FOUND); // should be 403
}

string ServerUtils::handleRedirect(const string &redirectUrl, int statusCode)
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

ResponseInfos ServerUtils::generateDirectoryListing(const string &dirPath)
{

    cout << "Im here and here is the path : " << dirPath << endl;
    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
    {
        return ressourceToResponse(ServerUtils::generateErrorPage(FORBIDEN), FORBIDEN);
    }

    struct dirent *entry;
    stringstream dirContent;

    dirContent << "<html><body><h1>Directory Listing for " << dirPath << "</h1><ul>";

    while ((entry = readdir(dir)) != nullptr)
    {
        // Skip . and .. directories
        // if (string(entry->d_name) == "." || string(entry->d_name) == "..")
        //     continue;

        dirContent << "<li><a href=\"" << entry->d_name << "\">" << entry->d_name << "</a></li>";
    }

    dirContent << "</ul></body></html>";
    closedir(dir);
    return ressourceToResponse(dirContent.str(), OK);
}

std::string ServerUtils::getMimeType(const std::string &filePath)
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

ResponseInfos ServerUtils::ressourceToResponse(string ressource, int code)
{
    ResponseInfos response_infos;
    response_infos.body = ressource;
    response_infos.status = code;
    response_infos.statusMessage = Request::generateStatusMsg(code);
    response_infos.headers["Content-Type"] = "text/html";
    response_infos.headers["Content-Length"] = to_string(ressource.length());

    return response_infos;
}

string ServerUtils::generateErrorPage(int statusCode)
{

    stringstream errorPage;
    errorPage << "<html><body><h1>" << statusCode << " " << "</h1></body></html>";
    return errorPage.str();
}

bool ServerUtils::isMethodAllowed(const std::string &method, const std::vector<std::string> &allowMethods)
{
    vector<string>::const_iterator it = allowMethods.begin();
    while (it != allowMethods.end())
    {
        if (*it == method)
        {
            return true;
        }
        it++;
    }
    return false;
}

ostream &operator<<(ostream &os, const ResponseInfos &response)
{
    os << "Status: \n"
       << response.status << " " << response.statusMessage << endl;
    os << "Headers: \n"
       << endl;
    for (const auto &header : response.headers)
    {
        os << header.first << ": " << header.second << endl;
    }
    os << "Body: \n"
       << endl;
    os << response.body << endl;
    return os;
}