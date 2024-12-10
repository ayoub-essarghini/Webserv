#pragma once
#include <string>
#include <map>

using namespace std;

class Request {
public:
    Request(const string& request);
    const string& getMethod() const;
    const string& getPath() const;
    const string& getVersion() const;
    const map<string, string>& getHeaders() const;

private:
    string method;
    string path;
    string version;
    map<string, string> headers;
    void parseRequest(const string& request);
};

