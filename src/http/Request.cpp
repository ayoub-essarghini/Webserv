#include "Request.hpp"
#include <sstream>
#include <algorithm>
#include <stdexcept>

Request::Request(const string& request) {
    parseRequest(request);
}

void Request::parseRequest(const string& request) {
   
    stringstream ss(request);
    string last;
    ss >> method >> path >> version >> last;
    if (!last.empty())
        throw runtime_error("Invalid HTTP request format");
    if (method.empty() || path.empty() || version.empty()) {
        throw runtime_error("Invalid HTTP request format");
    }

    string line;
    bool hostExist = false;
    while (getline(ss, line) && !line.empty()) {
        if (line.empty()) break;
        if (line.find("host:"))
            hostExist = true;
        size_t pos = line.find(':');
        if (pos != string::npos) {
            string header_name = line.substr(0, pos);
            string header_value = line.substr(pos + 1);
            headers[header_name] = header_value;
        }
    }
    // if (!hostExist)
    //     throw runtime_error("Host not exist !!");
    
}

const string& Request::getMethod() const {
    return method;
}

const string& Request::getPath() const {
    return path;
}

const string& Request::getVersion() const {
    return version;
}

const map<string, string>& Request::getHeaders() const {
    return headers;
}
