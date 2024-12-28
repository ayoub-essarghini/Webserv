#pragma once
#include <string>
#include <map>
#include <stdexcept>
#include "../config/Config.hpp"

using namespace std;

class Request
{
public:
    Request(const string &request);

    // Getters
    const string &getMethod() const;
    const string &getPath() const;
    const string &getDecodedPath() const;
    const string &getVersion() const;
    const map<string, string> &getHeaders() const;

    // Status handling
    bool isValid() const;
    string getStatusMessage() const;

private:
    string method;
    string path;
    string decoded_path;
    string version;
    string last;
    map<string, string> headers;

    // For handling request parsing and validation
    void parseRequest(const string &request);
    void validateHeaders();

    // For handling locations (if applicable)
    Location getLocation(Request &request);

    string validatePath(const string &path); // Validates the URL path for ASCII and hex
    bool isHexDigit(char c);               // check if char is hexDigit
    void trim(string &str);
    char hexToChar(char high, char low);
    // Status handling
    bool validRequest;
    string statusMessage;
};
