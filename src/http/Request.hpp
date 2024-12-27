#pragma once
#include <string>
#include <map>
#include "../src/config/Config.hpp"
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
    string last;
    map<string, string> headers;
    void parseRequest(const string& request);
    Location getLocation(Request& request);
    
};

