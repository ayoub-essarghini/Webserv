#pragma once
#include <string>
#include <map>
#include "Request.hpp"
#include "../utils/MyType.hpp"

using namespace std;



#define BAD_REQUEST 400
#define NOT_IMPLEMENTED 405
#define NOT_FOUND 404
#define URI_TOO_LONG 414
#define VERSION_NOT_SUPPORTED 505



class HttpParser {
public:
    HttpParser();
    Request parse(const string& data);
    string getMethod() const;
    string getUri() const;
    string getVersion() const;
    map<string, string> getHeaders() const;
    string getBody() const;
private:
    void parseRequestLine(const string& line);
    void parseHeader(const string& line);
    void parseBody(const string& body);
    void processLine(const string& line);
    void validateHeaders();
    string validatePath(const string& path);
    void validateMethod(const string& method);
    bool isHexDigit(char c);
    bool isBadUri(const string& uri);
    char hexToChar(char high, char low);
    bool isBadUriTraversal(const string& uri);
    map<string, string> parseParams(const string& query);
    void trim(string& str);

    State state;

    string method;
    string uri;
    string version;
    map<string, string> headers;
    map<string, string> query_params;
    string body;

};


