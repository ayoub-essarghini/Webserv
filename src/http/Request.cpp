#include "Request.hpp"
#include <sstream>
#include <iostream>
#include <cctype>
#include <algorithm>

Request::Request(const std::string &request)
{

    // std::string normalizedRequest = normalizeLineEndings(request);
    parseRequest(request);
    // validateHeaders();
}

void Request::parseRequest(const std::string &request)
{
    size_t pos = 0;

    if (request.empty())
    {
        throw std::runtime_error("400 Bad Request: Empty request");
    }

    // Parse request line
    parseRequestLine(request, pos);
    
    // Parse headers
    parseHeaders(request, pos);

    // Validate required headers
    validateHeaders();

    // Parse body for non-GET requests
    if (method != "GET" && headers.count("Content-Length") > 0)
    {
        parseBody(request, pos);
    }

    decoded_path = validatePath(path);
}

void Request::validateLineEndings(const std::string &request, size_t &pos)
{
    if (pos >= request.size() || request[pos] != '\r')
    {
        throw std::runtime_error("400 Bad Request: Malformed request");
    }

    pos++;

    if (pos >= request.size() || request[pos] != '\n')
    {
        throw std::runtime_error("400 Bad Request: Malformed request");
    }

    pos++;
}

void Request::parseRequestLine(const std::string &request, size_t &pos)
{
    size_t pos2 = 0;
    std::string line = extractToken(request, pos2, '\r');
    pos = pos2;
    // validateLineEndings(request, pos);
    char *check = (char *)line.c_str();
    bool valid = false;
    int i = 0;
    while (check[i])
    {
        if ((check[i] == ' ' && check[i + 1] == ' ') || (check[i] == '\t' && check[i + 1] == '\t') || (check[i] == ' ' && check[i + 1] == '\t') || (check[i] == '\t' && check[i + 1] == ' '))
            throw std::runtime_error("Expected continuous spaces");
        i++;
    }
    std::string last;
    std::stringstream ss(line);
    ss >> method >> path >> version >> last;

    validateMethod(method);

    if (!last.empty())
        throw std::runtime_error("malformed request");
    if (version != "HTTP/1.1")
        throw std::runtime_error("500 Unsupported HTTP version");

    std::cout << method << std::endl
              << path << std::endl
              << version << std::endl;

    if (path.find('?') != std::string::npos)
    {
        size_t queryStart = path.find('?');
        query_params = parseParams(path.substr(queryStart + 1));
        path = path.substr(0, queryStart);
    }

    // validateLineEndings(request, pos);
}

void Request::parseHeaders(const std::string &request, size_t &pos)
{
    while (pos < request.size())
    {
        // Check for end of headers (empty line)
        if (request[pos] == '\r')
        {
            if (pos + 1 >= request.size() || request[pos + 1] != '\n')
            {
                throw std::runtime_error("400 Bad Request: Expected CRLF");
            }
            pos += 2; // Skip the empty line
            break;
        }

        std::string line = extractToken(request, pos, '\r');
        
        size_t separator = line.find(':');
        if (separator == std::string::npos)
        {
            throw std::runtime_error("400 Bad Request: Malformed header");
        }

        std::string name = trim(line.substr(0, separator));
        std::string value = trim(line.substr(separator + 1));

        if (name.empty() || value.empty())
        {
            throw std::runtime_error("400 Bad Request: Empty header name or value");
        }

        headers[name] = value;
    }
}

void Request::parseBody(const std::string &request, size_t &pos)
{
    int contentLength = std::stoi(headers["Content-Length"]);
    if (pos + contentLength > request.size())
    {
        throw std::runtime_error("400 Bad Request: Incomplete body");
    }
    body = request.substr(pos, contentLength);
}

void Request::validateHeaders()
{
    if (!headers.count("Host"))
    {
        throw std::runtime_error("400 Bad Request: Missing Host header");
    }
}

std::string Request::extractToken(const std::string &request, size_t &pos, char delimiter)
{
    size_t start = pos;
    while (pos < request.size() && request[pos] != '\r')
    {
        pos++;
    }

    if (pos >= request.size())
    {
        throw std::runtime_error("400 Bad Request: Malformed request");
    }

    std::string token = request.substr(start, pos - start);
    
    // Skip \r\n sequence
    if (pos + 1 >= request.size() || request[pos + 1] != '\n')
    {
        throw std::runtime_error("400 Bad Request: Expected CRLF");
    }
    pos += 2; // Skip both \r and \n
    
    return token;
}

void Request::validateMethod(const std::string &method)
{
    static const std::set<std::string> allowedMethods = {"GET", "POST", "DELETE"};
    if (allowedMethods.find(method) == allowedMethods.end())
    {
        throw std::runtime_error("405 Method Not Allowed: Unsupported HTTP method");
    }
}

bool Request::isHexDigit(char c)
{
    return std::isxdigit(static_cast<unsigned char>(c));
}

char Request::hexToChar(char high, char low)
{
    int highVal = std::isdigit(high) ? high - '0' : std::toupper(high) - 'A' + 10;
    int lowVal = std::isdigit(low) ? low - '0' : std::toupper(low) - 'A' + 10;
    return static_cast<char>((highVal << 4) | lowVal);
}

std::string Request::trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}



bool Request::isBadUri(const std::string &uri)
{
    // RFC 3986 allowed characters in URLs
    static const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
    static const std::string gen_delims = ":/?#[]@";
    static const std::string sub_delims = "!$&'()*+,;=";
    static const std::string reserved = gen_delims + sub_delims;
    static const std::string allowed = unreserved + reserved;

    for (size_t i = 0; i < uri.length(); ++i)
    {
        char c = uri[i];

        // Allow percent-encoded characters
        if (c == '%')
        {
            if (i + 2 >= uri.length() ||
                !isxdigit(static_cast<unsigned char>(uri[i + 1])) ||
                !isxdigit(static_cast<unsigned char>(uri[i + 2])))
            {
                return true; // Invalid percent encoding
            }
            i += 2; // Skip the two hex digits
            continue;
        }

        // Check for control characters and non-ASCII characters
        if (c < 32 || c > 126)
        {
            return true;
        }

        // Check if character is in allowed set
        if (allowed.find(c) == std::string::npos)
        {
            return true;
        }




        // Special handling for fragment identifier (#)
        if (c == '#')
        {
            // Fragment must be the last part of the URI
            for (size_t j = i + 1; j < uri.length(); ++j)
            {
                char fc = uri[j];
                if (fc < 32 || fc > 126 || (fc != '#' && allowed.find(fc) == std::string::npos))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

std::string Request::validatePath(const std::string &path)
{
    if (path.empty())
    {
        throw std::runtime_error("400 Bad Request: Empty path");
    }

    if (path[0] != '/')
    {
        throw std::runtime_error("400 Bad Request: Path must start with '/'");
    }

    if (path.length() > 2048)
    {
        throw std::runtime_error("414 Request-URI Too Long");
    }

    // Check for bad URI characters
    if (isBadUri(path))
    {
        std::cout << "Is bad uri " << std::endl;
        throw std::runtime_error("400 Bad Request: Invalid URI characters");
    }

    // Check for path traversal
    if (isBadUriTraversal(path))
    {
        std::cout << "Is Traversal uri " << std::endl;

        throw std::runtime_error("400 Bad Request: Path traversal detected");
    }

    // Decode percent-encoded characters
    std::string decoded;
    for (size_t i = 0; i < path.size(); i++)
    {
        if (path[i] == '%')
        {
            if (i + 2 >= path.size())
            {
                throw std::runtime_error("400 Bad Request: Incomplete percent encoding");
            }

            if (!isHexDigit(path[i + 1]) || !isHexDigit(path[i + 2]))
            {
                throw std::runtime_error("400 Bad Request: Invalid percent encoding");
            }

            char decodedChar = hexToChar(path[i + 1], path[i + 2]);

            // Check if decoded character is valid
            if (decodedChar < 32 || decodedChar > 126)
            {
                throw std::runtime_error("400 Bad Request: Invalid decoded character");
            }

            decoded += decodedChar;
            i += 2;
        }
        else
        {
            decoded += path[i];
        }
    }

    return decoded;
}

// Updated isBadUriTraversal function
bool Request::isBadUriTraversal(const std::string &uri)
{
    std::string::size_type pos = 0;
    std::string::size_type prevPos = 0;

    while ((pos = uri.find('/', prevPos)) != std::string::npos)
    {
        if (pos == prevPos)
        {
            prevPos = pos + 1;
            continue;
        }

        std::string segment = uri.substr(prevPos, pos - prevPos);

        // Check for path traversal attempts
        if (segment == ".." ||
            segment == "." ||
            segment.find("../") != std::string::npos ||
            segment.find("./") != std::string::npos)
        {
            return true;
        }

        prevPos = pos + 1;
    }

    // Check the last segment
    if (prevPos < uri.length())
    {
        std::string lastSegment = uri.substr(prevPos);
        if (lastSegment == ".." ||
            lastSegment == "." ||
            lastSegment.find("../") != std::string::npos ||
            lastSegment.find("./") != std::string::npos)
        {
            return true;
        }
    }

    return false;
}

std::map<std::string, std::string> Request::parseParams(const std::string &query)
{
    std::map<std::string, std::string> params;

    std::string::const_iterator start = query.begin();
    std::string::const_iterator it = query.begin();
    std::string key, value;
    for (; it != query.end(); ++it)
    {
        if (*it == '=' && start != it)
        {
            key = trim(std::string(start, it));
            start = it + 1;
        }
        else if (*it == '&' && !key.empty())
        {
            value = trim(std::string(start, it));
            params[key] = value;
            start = it + 1;
            key.clear();
            value.clear();
        }
    }
    return params;
}

bool Request::isValid() const { return validRequest; }
std::string Request::getStatusMessage() const { return statusMessage; }
const std::string &Request::getMethod() const { return method; }
const std::string &Request::getPath() const { return path; }
const std::string &Request::getDecodedPath() const { return decoded_path; }
const std::string &Request::getVersion() const { return version; }
const std::map<std::string, std::string> &Request::getHeaders() const { return headers; }
const std::string &Request::getBody() const { return body; }
