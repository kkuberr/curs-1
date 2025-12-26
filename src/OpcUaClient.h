#pragma once
#include <memory>
#include <vector>
#include <string>
#include "IUaClient.h"  
#include "UaTypes.h"    

class OpcUaClient {
public:
    OpcUaClient();
    ~OpcUaClient();

    bool connect(const std::string& url);
    void disconnect();
    bool isConnected() const;

    std::vector<BrowseItem> browse_objects();
    ReadResult read_value(const std::string& nodeId);
    bool write_value(const std::string& nodeId, const std::string& value);

private:
    class Impl;
    std::unique_ptr<Impl> m_impl;
};
