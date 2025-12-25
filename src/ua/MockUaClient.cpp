#include "MockUaClient.h"

bool MockUaClient::connect(const std::string&) {
    m_connected = true;

    // Исправлено: инициализируем значения для тех же NodeId, 
    // которые возвращаются в browseObjects()
    m_values = {
        {"ns=2;i=1", "25"},
        {"ns=2;i=2", "1.2"},
        {"ns=2;i=3", "45"},
        {"ns=2;i=4", "1200"},
        {"ns=2;i=5", "10.5"},
        {"ns=2;i=6", "Active"},
        {"ns=2;i=7", "500"},
        {"ns=2;i=8", "22.1"},
        {"ns=2;i=9", "0.95"},
        {"ns=2;i=10", "Running"}
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
        {"ns=2;i=1", "Device1 / Temperature"},
        {"ns=2;i=2", "Device1 / Pressure"},
        {"ns=2;i=3", "Device1 / Humidity"},
        {"ns=2;i=4", "Device2 / Speed"},
        {"ns=2;i=5", "Device2 / Position"},
        {"ns=2;i=6", "Device3 / Status"},
        {"ns=2;i=7", "Device3 / Power"},
        {"ns=2;i=8", "Sensor1 / Reading"},
        {"ns=2;i=9", "Sensor2 / Value"},
        {"ns=2;i=10", "System / State"}
    };
}

ReadResult MockUaClient::readValue(const std::string& nodeId) {
    // Если nodeId нет в словаре, возвращаем дефолтное значение вместо пустого
    if (m_values.find(nodeId) != m_values.end()) {
        return { m_values[nodeId], "String" };
    }
    return { "0", "String" };
}

bool MockUaClient::writeValue(const std::string& nodeId,
                             const std::string& value) {
    m_values[nodeId] = value;
    return true;
}