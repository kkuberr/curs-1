#pragma once

#include <string>
#include <vector>

// Предварительное объявление
struct UA_Client;

struct BrowseItem {
    std::string display;
    std::string nodeid;
};

struct ReadResult {
    std::string value;
    std::string type;
};

class OpcUaClient {
public:
    OpcUaClient();
    ~OpcUaClient();

    bool connect(const std::string& url);
    void disconnect();
    bool isConnected() const;

    std::vector<BrowseItem> browse_objects();
    std::string find_node_by_display(const std::string& display) const;
    ReadResult read_value(const std::string& nodeid);
    bool write_value(const std::string& nodeid, const std::string& newValue);

private:
    bool m_connected;
    // Флаг: если true, используем заглушки вместо реального сервера
    bool m_isMockMode = false; 
    
    std::vector<BrowseItem> m_cached;
    UA_Client* m_clientPtr = nullptr;
};