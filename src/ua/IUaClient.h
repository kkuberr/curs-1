#pragma once
#include <string>
#include <vector>
#include "UaTypes.h"

class IUaClient {
public:
    virtual ~IUaClient() = default;

    virtual bool connect(const std::string& url) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;

    virtual std::vector<BrowseItem> browseObjects() = 0;
    virtual ReadResult readValue(const std::string& nodeId) = 0;
    virtual bool writeValue(const std::string& nodeId,
                            const std::string& value) = 0;
};
