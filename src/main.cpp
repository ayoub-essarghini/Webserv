#include "server/Server.hpp"
#include <iostream>

int main() {
    try {
        Config config("src/Server.conf");
        config.parse();
        Server server(8080,config);
        server.start();
     
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
