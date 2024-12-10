#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>


Config::Config(const string& config_file) : file_path(config_file), port(0) {}

void Config::parse() {
    ifstream file(file_path.c_str());
    if (!file.is_open())
        throw runtime_error("Failed to open configuration file: " + file_path);

    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') // Skip empty lines and comments
            continue;

        if (line.find("server {") == 0) {
            parseServer(file);
        } else {
            throw runtime_error("Unexpected token in configuration file: " + line);
        }
    }
}

void Config::parseServer(istream& stream) {
    string line;
    while (getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("listen ") == 0) {
            port = atoi(line.substr(7).c_str());
        } else if (line.find("root ") == 0) {
            root = line.substr(5);
        } else if (line.find("error_page ") == 0) {
            istringstream iss(line.substr(11));
            int code;
            string path;
            iss >> code >> path;
            error_pages[code] = path;
        } else if (line.find("location ") == 0) {
            size_t pos = line.find('{');
            if (pos == string::npos)
                throw runtime_error("Invalid location block syntax");

            string location_path = trim(line.substr(9, pos - 9));
            parseLocation(stream, location_path);
        } else if (line == "}") {
            break; // End of server block
        } else {
            throw runtime_error("Unknown directive in server block: " + line);
        }
    }
}

void Config::parseLocation(istream& stream, const string& location_path) {
    Location loc;
    string line;

    while (getline(stream, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("root ") == 0) {
            loc.root = line.substr(5);
        } else if (line.find("allow_methods ") == 0) {
            istringstream iss(line.substr(14));
            string method;
            while (iss >> method) {
                loc.allow_methods.push_back(method);
            }
        } else if (line.find("autoindex ") == 0) {
            loc.autoindex = (line.substr(10) == "on");
        } else if (line.find("index ") == 0) {
            istringstream iss(line.substr(6));
            string file;
            while (iss >> file) {
                loc.index_files.push_back(file);
            }
        } else if (line.find("client_max_body_size ") == 0) {
            loc.client_max_body_size = atoi(line.substr(21).c_str());
        } else if (line.find("upload_dir ") == 0) {
            loc.upload_dir = line.substr(11);
        } else if (line.find("return ") == 0) {
            loc.redirect = line.substr(7);
        } else if (line == "}") {
            locations[location_path] = loc;
            return; // End of location block
        } else {
            throw runtime_error("Unknown directive in location block: " + line);
        }
    }

    throw runtime_error("Unterminated location block");
}

int Config::getPort() const {
    return port;
}

const string& Config::getRoot() const {
    return root;
}

const Location& Config::getLocation(const string& path) const {
    map<string, Location>::const_iterator it = locations.find(path);
    if (it == locations.end())
        throw runtime_error("Location not found: " + path);
    return it->second;
}

string Config::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}
