#pragma once
#include "IUaClient.h"
#include <unordered_map>

class MockUaClient final : public IUaClient {
public:
    bool connect(const std::string&) override;
    void disconnect() override;
    bool isConnected() const override;

    std::vector<BrowseItem> browseObjects() override;
    ReadResult readValue(const std::string& nodeId) override;
    bool writeValue(const std::string& nodeId,
                    const std::string& value) override;

private:
    bool m_connected{false};
    std::unordered_map<std::string, std::string> m_values;
};
