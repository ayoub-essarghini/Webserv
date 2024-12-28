#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <cctype>
#include <algorithm>

Request::Request(const std::string& request) {
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

        std::cout << statusMessage << std::endl;
    }
}

void Request::parseRequest(const std::string& request) {
    size_t pos = 0;

    if (request.empty()) {
        throw std::runtime_error("400 Bad Request: Empty request");
    }

    parseRequestLine(request, pos);
    parseHeaders(request, pos);

    if (headers.count("Content-Length")) {
        parseBody(request, pos);
    }

    decoded_path = validatePath(path);
}

void Request::parseRequestLine(const std::string& request, size_t& pos) {
    method = extractToken(request, pos, ' ');
    validateMethod(method);

    path = extractToken(request, pos, ' ');
    version = extractToken(request, pos, '\r');

    if (version != "HTTP/1.1") {
        throw std::runtime_error("505 HTTP Version Not Supported");
    }

    validateLineTermination(request, pos);
}

void Request::parseHeaders(const std::string& request, size_t& pos) {
    while (pos < request.size() && request[pos] != '\r') {
        std::string line = extractToken(request, pos, '\r');
        validateLineTermination(request, pos);

        size_t separator = line.find(":");
        if (separator == std::string::npos) {
            throw std::runtime_error("400 Bad Request: Malformed header");
        }

        std::string name = trim(line.substr(0, separator));
        std::string value = trim(line.substr(separator + 1));

        if (name.empty() || value.empty()) {
            throw std::runtime_error("400 Bad Request: Empty header name or value");
        }

        headers[name] = value;
    }

    validateLineTermination(request, pos);
}

void Request::parseBody(const std::string& request, size_t& pos) {
    int contentLength = std::stoi(headers["Content-Length"]);
    if (pos + contentLength > request.size()) {
        throw std::runtime_error("400 Bad Request: Incomplete body");
    }
    body = request.substr(pos, contentLength);
}

void Request::validateHeaders() {
    if (!headers.count("Host")) {
        throw std::runtime_error("400 Bad Request: Missing Host header");
    }
}

std::string Request::extractToken(const std::string& request, size_t& pos, char delimiter) {
    size_t start = pos;
    while (pos < request.size() && request[pos] != delimiter) {
        pos++;
    }

    if (pos >= request.size()) {
        throw std::runtime_error("400 Bad Request: Malformed request");
    }

    std::string token = request.substr(start, pos - start);
    pos++;
    return token;
}

void Request::validateMethod(const std::string& method) {
    static const std::set<std::string> allowedMethods = {"GET", "POST", "DELETE"};
    if (allowedMethods.find(method) == allowedMethods.end()) {
        throw std::runtime_error("405 Method Not Allowed: Unsupported HTTP method");
    }
}

void Request::validateLineTermination(const std::string& request, size_t& pos) {
    if (pos + 1 >= request.size() || request[pos] != '\n') {
        throw std::runtime_error("400 Bad Request: Invalid line termination");
    }
    pos++;
}

std::string Request::validatePath(const std::string& path) {
    if (path.empty() || path[0] != '/') {
        throw std::runtime_error("400 Bad Request: Path must start with '/'");
    }

    // Check for bad URI characters
    if (isBadUri(path)) {
        throw std::runtime_error("400 Bad Request: Invalid URI characters");
    }

    // Check for path traversal (`..`)
    if (isBadUriTraversal(path)) {
        throw std::runtime_error("400 Bad Request: Path traversal detected in URI");
    }

    std::string decoded;
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] == '%') {
            if (i + 2 >= path.size() || !isHexDigit(path[i + 1]) || !isHexDigit(path[i + 2])) {
                throw std::runtime_error("400 Bad Request: Invalid percent encoding");
            }
            decoded += hexToChar(path[i + 1], path[i + 2]);
            i += 2;
        } else if (path[i] < 32 || path[i] > 126) {
            throw std::runtime_error("400 Bad Request: Invalid character in path");
        } else {
            decoded += path[i];
        }
    }

    if (path.length() > 2048) {
        throw std::runtime_error("414 Request-URI Too Long");
    }

    return decoded;
}

bool Request::isHexDigit(char c) {
    return std::isxdigit(static_cast<unsigned char>(c));
}

char Request::hexToChar(char high, char low) {
    int highVal = std::isdigit(high) ? high - '0' : std::toupper(high) - 'A' + 10;
    int lowVal = std::isdigit(low) ? low - '0' : std::toupper(low) - 'A' + 10;
    return static_cast<char>((highVal << 4) | lowVal);
}

std::string Request::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

bool Request::isValid() const { return validRequest; }
std::string Request::getStatusMessage() const { return statusMessage; }
const std::string& Request::getMethod() const { return method; }
const std::string& Request::getPath() const { return path; }
const std::string& Request::getDecodedPath() const { return decoded_path; }
const std::string& Request::getVersion() const { return version; }
const std::map<std::string, std::string>& Request::getHeaders() const { return headers; }
const std::string& Request::getBody() const { return body; }

bool Request::isBadUri(const std::string& uri) {
    // Check if URI contains only valid characters
    static const std::string allowedChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~:/?#[]@!$&'()*+,;=%";
    for (char c : uri) {
        if (allowedChars.find(c) == std::string::npos) {
            return true;
        }
    }
    return false;
}

bool Request::isBadUriTraversal(const std::string& uri) {
    // Check for path traversal (e.g., '/../')
    if (uri.find("/../") != std::string::npos || uri == ".." || uri.find("/..") == 0) {
        return true;
    }
    return false;
}
