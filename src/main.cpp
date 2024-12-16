#include "server/Server.hpp"
#include <iostream>

int main() {
    try {
        // Server server(8080);
        // server.start();
        Config config("src/Server.conf");
        config.parse();
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}
