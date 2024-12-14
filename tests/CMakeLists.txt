cmake_minimum_required(VERSION 3.10)
project(Test)

# Set GoogleTest directory to the location of your Google Test installation
# Adjust the path if you installed it elsewhere
set(GTEST_DIR $ENV{HOME}/.local)

# Add the include and lib directories for Google Test
include_directories(${GTEST_DIR}/include)
link_directories(${GTEST_DIR}/lib)

# Add the path to the src directory to include headers
include_directories(${CMAKE_SOURCE_DIR}/../src)

# Add the config source files (Config.cpp)
add_library(config STATIC ../src/config/Config.cpp)

# Create a test executable
add_executable(tester Test.cpp)

# Link Google Test, Google Test Main, and config to your test executable
target_link_libraries(tester gtest gtest_main config)

# Enable testing with CTest
enable_testing()
add_test(NAME TESTER COMMAND tester)