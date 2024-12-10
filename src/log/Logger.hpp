#pragma once

#include <string>

class Logger {
public:
    static void logInfo(const std::string& message);
    static void logError(const std::string& message);
};