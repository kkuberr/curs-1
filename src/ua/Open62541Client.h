#pragma once
#include <string>
#include <vector>
#include "OpcUaClient.h"

#ifdef WITH_OPEN62541
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/types.h>
#include <open62541/types_generated.h>
#endif

class Open62541Client : public IUaClient {
public:
    Open62541Client();
    ~Open62541Client() override;

    bool connect(const std::string& url) override;
    void disconnect() override;
    bool isConnected() const override;

    std::vector<BrowseItem> browseObjects() override;
    ReadResult readValue(const std::string& nodeId) override;
    bool writeValue(const std::string& nodeId, const std::string& value) override;

private:
#ifdef WITH_OPEN62541
    UA_Client* m_client;
#endif
    bool m_connected;
};
