#include "OpcUaClient.h"
#include "ua/MockUaClient.h"
#include "ua/Open62541Client.h"

class OpcUaClient::Impl {
public:
    std::unique_ptr<IUaClient> client;
};

OpcUaClient::OpcUaClient() : m_impl(std::make_unique<Impl>()) {}
OpcUaClient::~OpcUaClient() = default;

bool OpcUaClient::connect(const std::string& url) {
    // сначала пробуем реальный клиент
    auto real = std::make_unique<Open62541Client>();
    if (real->connect(url)) {
        m_impl->client = std::move(real);
        return true;
    }

    // fallback на Mock
    auto mock = std::make_unique<MockUaClient>();
    bool ok = mock->connect(url); // в mock можно игнорировать URL
    m_impl->client = std::move(mock);
    return ok;
}

void OpcUaClient::disconnect() {
    if (m_impl->client)
        m_impl->client->disconnect();
}

bool OpcUaClient::isConnected() const {
    return m_impl->client && m_impl->client->isConnected();
}

std::vector<BrowseItem> OpcUaClient::browse_objects() {
    if (!m_impl->client) return {};
    return m_impl->client->browseObjects();
}

ReadResult OpcUaClient::read_value(const std::string& nodeId) {
    if (!m_impl->client) return {"<error>", "-"};
    return m_impl->client->readValue(nodeId);
}

bool OpcUaClient::write_value(const std::string& nodeId, const std::string& value) {
    if (!m_impl->client) return false;
    return m_impl->client->writeValue(nodeId, value);
}
