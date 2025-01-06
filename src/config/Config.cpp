#include "Config.hpp"


Config::Config(const string &config_file) : file_path(config_file), port(0) {}

void Config::parse()
{
    parseServer();
    parseLocation();
    // print_config();
}

void Config::print_config() const
{
    cout <<"server root : " << root <<endl;
    cout << "server port "  << port << endl;
    cout << "server name " << server_name << endl;

    cout << "locations data :" << endl;
    map<string,Location>::const_iterator locatIt = locations.begin();

    while(locatIt!= locations.end())
    {
        cout << locatIt->first << "\n";
        cout << locatIt->second << endl;
        locatIt++;
    }
    cout << "error pages: " << endl;
    map<int,string>::const_iterator errorpagesIt = error_pages.begin();
    while (errorpagesIt != error_pages.end())
    {
        cout << errorpagesIt->first << errorpagesIt->second << endl;
        errorpagesIt++;
    }
    
}

void Config::parseServer()
{
    file_path = "";
    port = 8080;
    root = "www";
    server_name = "s1";
    error_pages[404] = "/404.html";
    error_pages[500] = "/500.html";

    index_files.push_back("index.html");
    index_files.push_back("index.htm");
    

}

void Config::parseLocation()
{
    Location l1;
    l1.root = "/www";
    // l1.allow_methods.push_back("GET");
    l1.allow_methods.push_back("POST");

    l1.autoindex = true;
    l1.index_files.push_back("index.html");
    l1.client_max_body_size = 1024;



    Location l2;
    l2.root = "/test/test2";
    l2.allow_methods.push_back("POST");
    l2.autoindex = false;
    l2.client_max_body_size = 1024 * 5;
    l2.upload_dir = "/www/upload";

    Location l3;
    l3.redirect = "http://example.com";
    l3.redirectCode = 301;

    locations["/test/"] = l1;
    locations["/test/test2/"] = l2;
    locations["/upload"] = l3;

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
    {
        static Location location;
        return location;
    }
    return it->second;
}

const map<string,Location> Config::getLocations()
{
    return locations;
}

vector<string> Config::getIndexFiles()
{
    return index_files;
}




ostream &operator<<(ostream& os,const Location& location)
{
    vector<string>::const_iterator it = location.allow_methods.begin();
    while (it != location.allow_methods.end())
    {
        cout << "method:  "<< *it << endl; 
        it++;
    }
    cout << "root: "<<location.root << endl;
    cout << "auto index : " << location.autoindex << endl;
    cout << "client max body size : " << location.client_max_body_size << endl;
    vector<string>::const_iterator it2 = location.index_files.begin();

        cout << "indexes: ";
    while (it2 != location.index_files.end())
    {
        cout << *it2 << " " << endl ;
        it2++;
    }
    cout << "redirect: " <<location.redirect << " " << endl;
    cout << "redirect code: " << location.redirectCode << endl;
    

    return os;
}