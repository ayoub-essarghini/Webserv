#pragma once

#include <string>
#include <iostream>

class Logger {
public:
    static void logInfo(const std::string& message);
    static void logError(const std::string& message);
};