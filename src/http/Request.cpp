#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cctype>
#include <algorithm>

Request::Request(const string& request) {
    validRequest = false;
    statusMessage = "Invalid HTTP request format";
    
    try {
        parseRequest(request);
        validateHeaders();
        validRequest = true;
        statusMessage = "Valid HTTP request";
    } catch (const std::runtime_error& e) {
        validRequest = false;
        statusMessage = e.what();

        cout << statusMessage << endl;
    }
}

void Request::parseRequest(const string& request) {
    size_t pos = 0;
    size_t len = request.length();
    
    // Step 1: Check for empty request
    if (len == 0) {
        throw std::runtime_error("400 Bad Request: Empty request");
    }

    // Parse request line
    parseRequestLine(request, pos, len);

    // Parse headers
    parseHeaders(request, pos, len);

    // Parse body if Content-Length header is present
    parseBody(request, pos, len);

    // Validate and decode the path
    decoded_path = validatePath(path);
}

void Request::parseRequestLine(const string& request, size_t& pos, size_t len) {
 
    method = parseMethod(request, pos, len);

    
    validateMethod(method);

    // Skip whitespace
    while (pos < len && request[pos] == ' ') pos++;

    // Parse path and query parameters (if any) 
    while (pos < len && request[pos] != ' ' && request[pos] != '?') {
        path += request[pos++];
    }

    // Check for query parameters (starts with '?')
    if (pos < len && request[pos] == '?') {
        pos++;  
        while (pos < len && request[pos] != ' ' && request[pos] != '#') {
            string param;
            // Get parameter name
            while (pos < len && request[pos] != '=' && request[pos] != '&' && request[pos] != ' ' && request[pos] != '#') {
                param += request[pos++];
            }
            if (param.empty()) {
                throw std::runtime_error("400 Bad Request: Empty query parameter name");
            }

            // Skip '='
            if (pos < len && request[pos] == '=') {
                pos++;
            }

            // Get parameter value
            string value;
            while (pos < len && request[pos] != '&' && request[pos] != ' ' && request[pos] != '#') {
                value += request[pos++];
            }

            query_param[param] = value;

            // Skip '&' (if more parameters exist)
            if (pos < len && request[pos] == '&') {
                pos++;
            }
        }
    }

    // Check for fragment (starts with '#')
    if (pos < len && request[pos] == '#') {
        pos++;  // skip the '#' character
        while (pos < len) {
            fragment += request[pos++];
        }
    }

    // Skip whitespace
    while (pos < len && request[pos] == ' ') pos++;

    // Parse HTTP version
    while (pos < len && request[pos] != '\r' && request[pos] != '\n') {
        version += request[pos++];
    }

    if (version != "HTTP/1.1") {
        throw std::runtime_error("505 HTTP Version Not Supported");
    }

    // Skip \r\n
    if (pos + 1 >= len || request[pos] != '\r' || request[pos + 1] != '\n') {
        throw std::runtime_error("400 Bad Request: Invalid request line termination");
    }
    pos += 2;
}

void Request::parseHeaders(const string& request, size_t& pos, size_t len) {
    string header_name;
    string header_value;
    bool parsing_name = true;

    while (pos < len) {
        // Check for end of headers (empty line)
        if (request[pos] == '\r' && pos + 1 < len && request[pos + 1] == '\n') {
            if (!header_name.empty()) {
                trim(header_value);
                headers[header_name] = header_value;
            }
            pos += 2;
            break;
        }

        // Handle line endings
        if (request[pos] == '\r' && pos + 1 < len && request[pos + 1] == '\n') {
            if (!header_name.empty()) {
                trim(header_value);
                headers[header_name] = header_value;
            }
            header_name.clear();
            header_value.clear();
            parsing_name = true;
            pos += 2;
            continue;
        }

        if (parsing_name) {
            if (request[pos] == ':') {
                parsing_name = false;
                pos++;
                // Skip whitespace after colon
                while (pos < len && request[pos] == ' ') pos++;
            } else {
                header_name += request[pos];
            }
        } else {
            header_value += request[pos];
        }
        pos++;
    }
}

void Request::parseBody(const string& request, size_t& pos, size_t len) {
    if (headers.find("Content-Length") != headers.end()) {
        int content_length = std::atoi(headers["Content-Length"].c_str());
        if (content_length > 0) {
            if (pos + content_length > len) {
                throw std::runtime_error("400 Bad Request: Incomplete body");
            }
            body = request.substr(pos, content_length);
        }
    }
}

string Request::parseMethod(const string& request, size_t& pos, size_t len) {
    string method;
    while (pos < len && request[pos] != ' ') {
        method += request[pos++];
    }
    return method;
}

void Request::validateMethod(const string& method) {
    if (method.empty()) {
        throw std::runtime_error("400 Bad Request: Missing HTTP method");
    }

  
    if (method != "GET" && method != "POST" && method != "DELETE") {
        throw std::runtime_error("405 Method Not Allowed: Unsupported HTTP method");
    }
}

string Request::validatePath(const string& path) {
    string decoded;
    size_t i = 0;
    const size_t path_len = path.length();

  
    if (path.empty() || path[0] != '/') {
        throw std::runtime_error("400 Bad Request: Path must start with '/'");
    }

    while (i < path_len) {
        char c = path[i];

     
        if (c == '%') {
            if (i + 2 >= path_len) {
                throw std::runtime_error("400 Bad Request: Incomplete percent encoding");
            }

            char high = path[i + 1];
            char low = path[i + 2];

            if (!isHexDigit(high) || !isHexDigit(low)) {
                throw std::runtime_error("400 Bad Request: Invalid percent encoding");
            }

            char decoded_char = hexToChar(high, low);

          
            if (decoded_char < 32 || decoded_char == 127) {
                throw std::runtime_error("400 Bad Request: Invalid character in path");
            }

            decoded += decoded_char;
            i += 3;
            continue;
        }

     
        if (c < 32 || c > 126) {
            throw std::runtime_error("400 Bad Request: Invalid character in path");
        }

        decoded += c;
        i++;
    }

    return decoded;
}

bool Request::isHexDigit(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

char Request::hexToChar(char high, char low) {
    high = toupper(high);
    low = toupper(low);

    int highVal = (high >= 'A') ? (high - 'A' + 10) : (high - '0');
    int lowVal = (low >= 'A') ? (low - 'A' + 10) : (low - '0');

    return static_cast<char>((highVal << 4) | lowVal);
}

void Request::trim(string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");

    if (start == string::npos || end == string::npos) {
        str.clear();
    } else {
        str = str.substr(start, end - start + 1);
    }
}

void Request::validateHeaders() {
    if (headers.find("Host") == headers.end()) {
        throw std::runtime_error("400 Bad Request: Missing Host header");
    }

    if (headers.count("Host") > 1) {
        throw std::runtime_error("400 Bad Request: Multiple Host headers");
    }
}

bool Request::isValid() const { return validRequest; }
string Request::getStatusMessage() const { return statusMessage; }

const string& Request::getMethod() const {
    return method;
}

const string& Request::getPath() const {
    return path;
}

const string& Request::getDecodedPath() const {
    return decoded_path;
}

const string& Request::getVersion() const {
    return version;
}

const map<string, string>& Request::getHeaders() const {
    return headers;
}

const string& Request::getBody() const {
    return body;
}