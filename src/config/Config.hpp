#pragma once
#include <iostream>
#include <map>
#include <vector>
using namespace std;
struct Location {
    string root;
    vector<string> allow_methods;
    bool autoindex;
    vector<string> index_files;
    size_t client_max_body_size;
    string upload_dir;
    string redirect;
    int redirectCode;
    Location() : autoindex(false), client_max_body_size(0) {}
};

class Config {
public:
    Config(const string& config_file);
    void parse();
    int getPort() const;
    const string& getRoot() const;
    const Location& getLocation(const string& path) const;
    void print_config() const;

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

ostream &operator<<(ostream& os,const Location& location);


