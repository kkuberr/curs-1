#include "Open62541Client.h"
#include <iostream>
#include <sstream>

#ifdef WITH_OPEN62541
// Include open62541 C headers with C linkage for C++ compilation
extern "C" {
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/types.h>
#include <open62541/types_generated.h>
}

#endif


#ifdef WITH_OPEN62541
#include <open62541/client_highlevel.h>
static std::string uaStringToStd(const UA_String& str) {
    return std::string(reinterpret_cast<const char*>(str.data), str.length);
}

static std::string nodeIdToString(const UA_NodeId& nodeId) {
    std::ostringstream oss;

    if (nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
        oss << "ns=" << nodeId.namespaceIndex
            << ";i=" << nodeId.identifier.numeric;
    }
    else if (nodeId.identifierType == UA_NODEIDTYPE_STRING) {
        oss << "ns=" << nodeId.namespaceIndex
            << ";s=" << uaStringToStd(nodeId.identifier.string);
    }
    else {
        oss << "<unsupported>";
    }

    return oss.str();
}

#endif

Open62541Client::Open62541Client()
    : m_connected(false)
#ifdef WITH_OPEN62541
    , m_client(nullptr)
#endif
{
#ifdef WITH_OPEN62541
    m_client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(m_client));
#endif
}

Open62541Client::~Open62541Client() {
    disconnect();
#ifdef WITH_OPEN62541
    if (m_client) {
        UA_Client_delete(m_client);
        m_client = nullptr;
    }
#endif
}

bool Open62541Client::connect(const std::string& url) {
#ifdef WITH_OPEN62541
    if (!m_client) return false;
    UA_StatusCode ret = UA_Client_connect(m_client, url.c_str());
    m_connected = (ret == UA_STATUSCODE_GOOD);
    return m_connected;
#else
    return false;
#endif
}

void Open62541Client::disconnect() {
#ifdef WITH_OPEN62541
    if (m_client && m_connected) {
        UA_Client_disconnect(m_client);
    }
#endif
    m_connected = false;
}

bool Open62541Client::isConnected() const {
    return m_connected;
}

std::vector<BrowseItem> Open62541Client::browseObjects() {
    std::vector<BrowseItem> result;

#ifdef WITH_OPEN62541
    if (!m_connected || !m_client) return result;

    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);

    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;

    bReq.nodesToBrowse[0].nodeId =
        UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    bReq.nodesToBrowse[0].resultMask =
        UA_BROWSERESULTMASK_NODECLASS |
        UA_BROWSERESULTMASK_DISPLAYNAME;

    UA_BrowseResponse bResp =
        UA_Client_Service_browse(m_client, bReq);

    if (bResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        for (size_t i = 0; i < bResp.resultsSize; ++i) {
            for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
                const UA_ReferenceDescription& ref =
                    bResp.results[i].references[j];

                if (ref.nodeClass != UA_NODECLASS_VARIABLE)
                    continue;

                BrowseItem item;
                item.nodeId = nodeIdToString(ref.nodeId.nodeId);
                item.displayPath = uaStringToStd(ref.displayName.text);

                result.push_back(item);
            }
        }
    }

    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);
#endif

    return result;
}

ReadResult Open62541Client::readValue(const std::string& nodeId) {
    ReadResult r{ "<error>", "-" };

#ifdef WITH_OPEN62541
    if (!m_connected || !m_client) return r;

    UA_NodeId nid;
    UA_String nodeIdStr = UA_STRING_ALLOC(nodeId.c_str());
    UA_NodeId_parse(&nid, nodeIdStr);
    UA_String_clear(&nodeIdStr);

    UA_Variant value;
    UA_Variant_init(&value);

    UA_StatusCode ret =
        UA_Client_readValueAttribute(m_client, nid, &value);

    if (ret != UA_STATUSCODE_GOOD) {
        UA_NodeId_clear(&nid);
        return r;
    }

    if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_BOOLEAN])) {
        r.value = (*(UA_Boolean*)value.data) ? "true" : "false";
        r.type = "Boolean";
    }
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_INT32])) {
        r.value = std::to_string(*(UA_Int32*)value.data);
        r.type = "Int32";
    }
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_DOUBLE])) {
        r.value = std::to_string(*(UA_Double*)value.data);
        r.type = "Double";
    }
    else if (UA_Variant_hasScalarType(&value, &UA_TYPES[UA_TYPES_STRING])) {
        r.value = uaStringToStd(*(UA_String*)value.data);
        r.type = "String";
    }
    else {
        r.value = "<unsupported>";
        r.type = "Other";
    }

    UA_Variant_clear(&value);
    UA_NodeId_clear(&nid);
#endif

    return r;
}

bool Open62541Client::writeValue(
    const std::string& nodeId,
    const std::string& value
) {
#ifdef WITH_OPEN62541
    if (!m_connected || !m_client) return false;
    UA_String nodeIdStr = UA_STRING_ALLOC(nodeId.c_str());

    UA_NodeId nid;
    UA_NodeId_parse(&nid, nodeIdStr);
    UA_String_clear(&nodeIdStr);

    UA_Variant v;
    UA_Variant_init(&v);

    try {
        double d = std::stod(value);
        UA_Variant_setScalarCopy(
            &v, &d, &UA_TYPES[UA_TYPES_DOUBLE]
        );
    }
    catch (...) {
        UA_String str = UA_STRING_ALLOC(value.c_str());
        UA_Variant_setScalarCopy(
            &v, &str, &UA_TYPES[UA_TYPES_STRING]
        );
        UA_String_clear(&str);
    }

    UA_StatusCode ret =
        UA_Client_writeValueAttribute(m_client, nid, &v);

    UA_Variant_clear(&v);
    UA_NodeId_clear(&nid);

    return ret == UA_STATUSCODE_GOOD;
#else
    return false;
#endif
}
