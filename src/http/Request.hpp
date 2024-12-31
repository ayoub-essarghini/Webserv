#pragma once

#include <string>
#include <map>
#include <set>

class Request
{
public:
    // Constructor
    Request(const std::string &raw_request);

    // Accessors for request components
    const std::string &getMethod() const;
    const std::string &getPath() const;
    const std::string &getDecodedPath() const;
    const std::string &getVersion() const;
    const std::map<std::string, std::string> &getHeaders() const;
    const std::string &getBody() const;

    // Status accessors
    bool isValid() const;
    std::string getStatusMessage() const;

private:
    // Request components
    std::string method;
    std::string path;
    std::string decoded_path;
    std::string version;
    std::map<std::string, std::string> headers;
    std::string body;

    // Status flags
    bool validRequest;
    std::string statusMessage;

    // Parsing methods
    void parseRequest(const std::string &request);
    void parseRequestLine(const std::string &request, size_t &pos);
    void parseHeaders(const std::string &request, size_t &pos);
    void parseBody(const std::string &request, size_t &pos);
    void validateHeaders();
    bool isBadUriTraversal(const std::string &uri);
    bool isBadUri(const std::string &uri);
    // Utility methods
    std::string extractToken(const std::string &request, size_t &pos, char delimiter);
    std::string validatePath(const std::string &path);
    void validateMethod(const std::string &method);
    void validateLineEndings(const std::string &request, size_t &pos);
    bool isHexDigit(char c);
    char hexToChar(char high, char low);
    std::string normalizeLineEndings(const std::string &request);
    std::string toLower(const std::string &str);
    std::string trim(const std::string &str);
};
