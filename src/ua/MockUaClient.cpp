#include "MockUaClient.h"

bool MockUaClient::connect(const std::string&) {
    m_connected = true;

    m_values = {
        {"ns=2;s=Device1.Temp", "25"},
        {"ns=2;s=Device1.Pressure", "1.2"},
        {"ns=2;s=Device2.Speed", "1200"}
    };
    return true;
}

void MockUaClient::disconnect() {
    m_connected = false;
}

bool MockUaClient::isConnected() const {
    return m_connected;
}

std::vector<BrowseItem> MockUaClient::browseObjects() {
    return {
        {"ns=2;s=Device1.Temp", "Device1 / Temp"},
        {"ns=2;s=Device1.Pressure", "Device1 / Pressure"},
        {"ns=2;s=Device2.Speed", "Device2 / Speed"}
    };
}

ReadResult MockUaClient::readValue(const std::string& nodeId) {
    return { m_values[nodeId], "Int32" };
}

bool MockUaClient::writeValue(const std::string& nodeId,
                             const std::string& value) {
    m_values[nodeId] = value;
    return true;
}
