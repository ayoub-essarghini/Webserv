#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

using namespace std;


class Request {
public:
    // Constructor to parse HTTP request
    Request(const string& raw_request);

    // Request line getters
    const string& getMethod() const;
    const string& getPath() const;
    const string& getDecodedPath() const;
    const string& getVersion() const;
    const map<string, string>& getHeaders() const;
    const string& getBody() const;

    // Status checks
    bool isValid() const;
    string getStatusMessage() const;

private:
    // Request components
    string method;             // HTTP method (GET, POST, etc.)
    string path;               // Raw URL path
    string fragment;           // URL fragment
    string decoded_path;       // Decoded URL path
    string version;            // HTTP version
    map<string, string> headers;  // Request headers
    map<string, string> query_param;  // Query parameters
    string body;               // Request body

    // Status flags and messages
    bool validRequest;
    string statusMessage;

    // Parsing functions
    void parseRequest(const string& request);
    void parseRequestLine(const string& request, size_t& pos, size_t len);
    void parseHeaders(const string& request, size_t& pos, size_t len);
    void parseBody(const string& request, size_t& pos, size_t len);
    void validateHeaders();
    
    // URL and path processing
    string validatePath(const string& path);
    
    bool isHexDigit(char c);
    char hexToChar(char high, char low);
    
    void trim(string& str);

    // Functions for HTTP method handling
    string parseMethod(const string& request, size_t& pos, size_t len);
    void validateMethod(const string& method);
};



