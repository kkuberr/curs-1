#pragma once
#include <string>

struct BrowseItem {
    std::string nodeId;
    std::string displayPath;   // Device / Var
};

struct ReadResult {
    std::string value;
    std::string type;
};
