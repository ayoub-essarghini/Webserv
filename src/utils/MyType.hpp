#pragma once 
#include <iostream>
#include <vector>
enum File_Type
{
    DIRECTORY,
    REGULAR,
    NOT_EXIST,
    UNDEFINED
};

enum State {
    REQUEST_LINE,
    HEADER,
    BODY
};

struct RessourceInfo
{
    std::string path;
    std::string url;
    std::string root;
    std::vector<std::string> indexFiles;
    bool autoindex;
};