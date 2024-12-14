#include "../src/config/Config.hpp"
#include <gtest/gtest.h>
#include <sstream>
#include <stdexcept>

using namespace std;

class ConfigTest : public ::testing::Test {
protected:
    // Setup before each test
    void SetUp() override {
        // You can initialize the object here if needed, e.g., an in-memory config
    }

    // You can also add a helper function to simulate reading from a file
    void setConfigFromString(Config& config, const string& config_str) {
        istringstream stream(config_str);
        config.parse();  // Call your parse method here
    }
};

// Test for a valid configuration with a simple server block
TEST_F(ConfigTest, Serverconfig) {
    string config_str = R"(
        server {
            listen 8080;
            root /var/www;
            error_page 404 /404.html;
            location / {
                root /var/www/html;
                allow_methods GET POST;
                autoindex on;
                index index.html index.htm;
            }
        }
    )";
    
    Config config("test_config");  // Create the config object (it could be a dummy filename)
    istringstream stream(config_str);
    config.parse();  // Manually call the parser here since we're using a string as input
    
    // Test that the port was parsed correctly
    EXPECT_EQ(config.getPort(), 8080);
    EXPECT_EQ(config.getRoot(), "/var/www");
    
    // Test that location parsing works
    const Location& location = config.getLocation("/");
    EXPECT_EQ(location.root, "/var/www/html");
    EXPECT_TRUE(location.autoindex);
    EXPECT_EQ(location.index_files.size(), 2);
    EXPECT_EQ(location.index_files[0], "index.html");
    EXPECT_EQ(location.index_files[1], "index.htm");
}

// Test for an invalid configuration file (e.g., missing semicolon after listen)
TEST_F(ConfigTest, TestInvalidConfig) {
    string config_str = R"(
        server {
            listen 8080
            root /var/www;
        }
    )";

    Config config("test_config");
    istringstream stream(config_str);

    // Expecting an exception due to the missing semicolon after 'listen'
    EXPECT_THROW(config.parse(), runtime_error);
}

// Test for a configuration with an unrecognized directive
TEST_F(ConfigTest, TestUnknownDirective) {
    string config_str = R"(
        server {
            listen 8080;
            root /var/www;
            unknown_directive value;
        }
    )";

    Config config("test_config");
    istringstream stream(config_str);

    // Expecting an exception due to the unknown directive
    EXPECT_THROW(config.parse(), runtime_error);
}

// Test for division by zero
TEST_F(ConfigTest, TestEmptyConfig) {
    string config_str = "";
    Config config("test_config");
    istringstream stream(config_str);
    
    // Expecting an exception due to empty config
    EXPECT_THROW(config.parse(), runtime_error);
}

// Test for a server block without a closing brace
TEST_F(ConfigTest, TestUnterminatedServerBlock) {
    string config_str = R"(
        server {
            listen 8080;
            root /var/www;
    )";

    Config config("test_config");
    istringstream stream(config_str);
    
    // Expecting an exception due to the unterminated server block
    EXPECT_THROW(config.parse(), runtime_error);
}
