#include "Request.hpp"
#include <sstream>
#include <stdexcept>
#include <string>
#include <cctype> 

Request::Request(const string& request){
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
    std::stringstream ss(request);
    ss >> method >> path >> version >> last;

    // Parse the request line
    if (method.empty() || path.empty() || version.empty()) {
        cout << "empty" << endl;
        throw runtime_error("Invalid HTTP request line format");
    }

    // Validate HTTP version
    if (version.substr(0, 5) != "HTTP/" || version != "HTTP/1.1") {
        throw runtime_error("Unsupported HTTP version");
    }

    if (!last.empty())
        throw runtime_error("Invalid HTTP request line format");

    // Validate path

    decoded_path =  validatePath(path).c_str();

    cout << "decoded path: "<<decoded_path;

    // Parse headers
    string line;
    while (std::getline(ss, line)) {
        if (line.empty()) break;

        size_t pos = line.find(':');
        if (pos != string::npos) {
            string header_name = line.substr(0, pos);
            string header_value = line.substr(pos + 1);

            // Trim spaces from header name and value
            trim(header_name);
            trim(header_value);

            headers[header_name] = header_value;
        } else {
            throw std::runtime_error("Invalid header format");
        }
    }
}

// Function to validate path for ASCII and hex encoding
string Request::validatePath(const string& path) {
      string decodedPath;
    size_t i = 0;
    while (i < path.size()) {
        char c = path[i];

        // Check for valid ASCII characters (printable + '%')
        if ((c < 32 || c > 126) && c != '%') {
            throw std::runtime_error("Path contains non-ASCII characters");
        }

        // If we encounter a '%', try to decode the following two characters
        if (c == '%') {
            if (i + 2 >= path.size() || !isHexDigit(path[i + 1]) || !isHexDigit(path[i + 2])) {
                throw std::runtime_error("Invalid hexadecimal encoding in path");
            }

            // Decode and append the character
            decodedPath += hexToChar(path[i + 1], path[i + 2]);
            i += 3; // Skip the '%' and the two hex digits
        } else {
            // If it's not '%', just append the character as is
            decodedPath += c;
            i++;
        }
    }

    // Now path contains the decoded path
    return decodedPath;
}

bool Request::isHexDigit(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

// Helper function to convert two hex digits to a character
char Request::hexToChar(char high, char low) {
    int highValue = (high >= 'A' ? high - 'A' + 10 : high - '0');
    int lowValue = (low >= 'A' ? low - 'A' + 10 : low - '0');
    return (highValue << 4) | lowValue; // Combine high and low nibble
}


// Helper function to trim leading and trailing spaces from a string
void Request::trim(string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");

    if (start == string::npos || end == string::npos) {
        str = ""; // Empty string
    } else {
        str = str.substr(start, end - start + 1);
    }
}

void Request::validateHeaders() {
    // Ensure the Host header exists
    if (headers.find("Host") == headers.end()) {
        throw std::runtime_error("Missing Host header");
    }
}

// Placeholder for Location function (adjust as necessary)
Location Request::getLocation(Request& request) {

    return Location(); // Placeholder implementation
}

// Getter methods
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

bool Request::isValid() const {
    return validRequest;
}

string Request::getStatusMessage() const {
    return statusMessage;
}
