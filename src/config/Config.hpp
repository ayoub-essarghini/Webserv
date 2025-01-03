#pragma once
#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include "../utils/MyType.hpp"

class Config
{
public:
    Config(const string &config_file);
    void parse();
    int getPort() const;
    const string &getRoot() const;
    const Location &getLocation(const string &path) const;
    const map<string, Location> getLocations();
    void print_config() const;
    vector<string> getIndexFiles();

private:
    string file_path;
    int port;
    string root;
    string server_name;
    map<int, string> error_pages;
    map<string, Location> locations;
    vector<string> index_files;

    void parseServer();
    void parseLocation();
};

ostream &operator<<(ostream &os, const Location &location);
