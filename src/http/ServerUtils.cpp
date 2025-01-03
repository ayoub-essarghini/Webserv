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

string ServerUtils::serveFile(const string &filePath)
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

string ServerUtils::serverRootOrRedirect(RessourceInfo ressource)
{
    if (ressource.url[ressource.url.length() - 1] != '/' && ressource.url != "/")
    {
        string redirectUrl = ressource.url + "/";
        return handleRedirect(redirectUrl, 301);
    }
    vector<string>::const_iterator inedxIter = ressource.indexFiles.begin();
    for (; inedxIter != ressource.indexFiles.end(); inedxIter++)
    {
        string indexPath = "src/" + ressource.root + "/" + *inedxIter;
        cout << "--------here-------\n";
        cout << indexPath << "\n";
        cout << "---------------\n";
        struct stat indexStat;
        if (stat(indexPath.c_str(), &indexStat) == 0)
        {
            return ServerUtils::serveFile(indexPath);
        }
    }
    if (ressource.autoindex)
        return ServerUtils::generateDirectoryListing(ressource.url);
    return ServerUtils::serveFile("src/www/404.html"); // should be 403
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

string ServerUtils::generateDirectoryListing(const string &dirPath)
{
    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
    {
        return ServerUtils::generateErrorPage(403);
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

string ServerUtils::generateErrorPage(int statusCode)
{

    stringstream errorPage;
    errorPage << "<html><body><h1>" << statusCode << " " << "</h1></body></html>";
    return errorPage.str();
}