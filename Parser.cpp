#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stack>
#include <unordered_set>
#include <vector>
#include <regex>

bool isDirectiveValid(const std::string& directive) {
    // List of valid Nginx directives (simplified for illustration)
    static std::unordered_set<std::string> validDirectives = {
        "worker_processes", "server", "location", "listen", "root"
    };

    return validDirectives.find(directive) != validDirectives.end();
}

bool parseConfFile(const std::string& filePath) {
    std::ifstream file(filePath);
    std::string line;
    std::stack<std::string> blockStack; // To track block nesting (e.g., server, location)
    bool inBlock = false;
    bool validSyntax = true;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;

        // Ignore empty lines and comments (lines starting with #)
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Tokenize the line by spaces (basic tokenization for simplicity)
        ss >> token;
        
        // If the token is a valid directive, check its correctness
        if (isDirectiveValid(token)) {
            std::cout << "Directive found: " << token << std::endl;
            // Validate additional parameters based on directive type
            // For example: worker_processes should be an integer
            if (token == "worker_processes") {
                int num;
                ss >> num;
                if (num <= 0) {
                    std::cerr << "Invalid value for worker_processes: " << num << std::endl;
                    validSyntax = false;
                }
            }
        } else if (token == "server" || token == "location") {
            // Starting a new block, push it onto the stack
            blockStack.push(token);
            inBlock = true;
            std::cout << "Entering " << token << " block." << std::endl;
        } else if (token == "}" && !blockStack.empty()) {
            // Ending a block, pop from the stack
            blockStack.pop();
            std::cout << "Exiting block." << std::endl;
        } else {
            std::cerr << "Unknown directive or invalid syntax: " << token << std::endl;
            validSyntax = false;
        }
    }

    // After parsing, check if all blocks were closed properly
    if (!blockStack.empty()) {
        std::cerr << "Unmatched block(s) detected!" << std::endl;
        validSyntax = false;
    }

    return validSyntax;
}

int main() {
    std::string filePath = "nginx.conf";
    if (parseConfFile(filePath)) {
        std::cout << "Configuration file is valid!" << std::endl;
    } else {
        std::cout << "Configuration file has errors!" << std::endl;
    }
    return 0;
}
