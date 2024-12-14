#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

Config::Config(const string &config_file) : file_path(config_file), port(0) {}

void Config::parse()
{
    ifstream file(file_path);
    if (!file.is_open())
    {
        cerr << "Unable to open file" << endl;
        return;
    }

    string line;
    while (getline(file, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("server") == 0)
        {
            parseServer(file);
        }
    }
    print_config();
}

void Config::print_config() const
{
    cout << "listen: " << port << endl;
    cout << "Root: " << root << endl;
    for (const auto &location : locations)
    {
        cout << "Location: " << location.first << endl;
        cout << "Root: " << location.second.root << endl;
    }
}

void Config::parseServer(istream &stream)
{
    string line;
    while (getline(stream, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("location") == 0)
        {
            string location_path = line.substr(8); 
            parseLocation(stream, trim(location_path));
        }
        else if (line.find("listen") == 0)
        {
            port = stoi(line.substr(7)); 
        }
        else if (line.find("root") == 0)
        {
            root = trim(line.substr(4)); 
        }
        else if (line.find("}") == 0)
        {
            break;
        }
    }
}

void Config::parseLocation(istream &stream, const string &location_path)
{
    Location location;
    string line;
    while (getline(stream, line))
    {
        line = trim(line);
        if (line.empty() || line[0] == '#')
            continue;

        if (line.find("root") == 0)
        {
            location.root = trim(line.substr(4));
        }
        else if (line.find("}") == 0)
        {
            break;
        }
    }
    locations[location_path] = location;
}

int Config::getPort() const
{
    return port;
}

const string &Config::getRoot() const
{
    return root;
}

const Location &Config::getLocation(const string &path) const
{
    map<string, Location>::const_iterator it = locations.find(path);
    if (it == locations.end())
        throw runtime_error("Location not found: " + path);
    return it->second;
}

string Config::trim(const string &str)
{
    size_t first = str.find_first_not_of(" \t");
    if (first == string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t");
    // if (last == ';' || last == '}')
    //     last--;
    return str.substr(first, last - first);
}
