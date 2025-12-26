#include "Open62541Client.h"
#include <iostream>
#include <sstream>

#ifdef WITH_OPEN62541
extern "C" {
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#include <open62541/types.h>
#include <open62541/types_generated.h>
#include <open62541/client_highlevel.h>
}

static std::string uaStringToStd(const UA_String& str) {
    if (str.length == 0) return "";
    return std::string(reinterpret_cast<const char*>(str.data), str.length);
}

static std::string nodeIdToString(const UA_NodeId& nodeId) {
    std::ostringstream oss;
    if (nodeId.identifierType == UA_NODEIDTYPE_NUMERIC) {
        oss << "ns=" << nodeId.namespaceIndex << ";i=" << nodeId.identifier.numeric;
    } else if (nodeId.identifierType == UA_NODEIDTYPE_STRING) {
        oss << "ns=" << nodeId.namespaceIndex << ";s=" << uaStringToStd(nodeId.identifier.string);
    } else {
        oss << "<unsupported>";
    }
    return oss.str();
}
#endif

Open62541Client::Open62541Client() : m_connected(false) {
#ifdef WITH_OPEN62541
    m_client = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(m_client));
#endif
}

Open62541Client::~Open62541Client() {
    disconnect();
#ifdef WITH_OPEN62541
    if (m_client) { UA_Client_delete(m_client); m_client = nullptr; }
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
    if (m_client && m_connected) { UA_Client_disconnect(m_client); }
#endif
    m_connected = false;
}

bool Open62541Client::isConnected() const { return m_connected; }

std::vector<BrowseItem> Open62541Client::browseObjects() {
    std::vector<BrowseItem> result;
#ifdef WITH_OPEN62541
    if (!m_connected || !m_client) return result;

    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); 
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; 

    UA_BrowseResponse bResp = UA_Client_Service_browse(m_client, bReq);

    for (size_t i = 0; i < bResp.resultsSize; ++i) {
        for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
            UA_ReferenceDescription *ref = &bResp.results[i].references[j];
            
            if (ref->nodeId.nodeId.namespaceIndex == 0) continue;

            BrowseItem item;
            item.nodeId = nodeIdToString(ref->nodeId.nodeId);
            item.displayPath = uaStringToStd(ref->browseName.name);
            
            if (item.displayPath != "Server") {
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
    UA_StatusCode ret = UA_Client_readValueAttribute(m_client, nid, &value);

    if (ret == UA_STATUSCODE_GOOD && UA_Variant_isScalar(&value)) {
        if (value.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
            r.value = (*(UA_Boolean*)value.data) ? "true" : "false"; r.type = "Boolean";
        } else if (value.type == &UA_TYPES[UA_TYPES_INT16]) {
            r.value = std::to_string(*(UA_Int16*)value.data); r.type = "Int16";
        } else if (value.type == &UA_TYPES[UA_TYPES_INT32]) {
            r.value = std::to_string(*(UA_Int32*)value.data); r.type = "Int32";
        } else if (value.type == &UA_TYPES[UA_TYPES_INT64]) {
            r.value = std::to_string(*(UA_Int64*)value.data); r.type = "Int64";
        } else if (value.type == &UA_TYPES[UA_TYPES_UINT16]) {
            r.value = std::to_string(*(UA_UInt16*)value.data); r.type = "UInt16";
        } else if (value.type == &UA_TYPES[UA_TYPES_UINT32]) {
            r.value = std::to_string(*(UA_UInt32*)value.data); r.type = "UInt32";
        } else if (value.type == &UA_TYPES[UA_TYPES_FLOAT]) {
            r.value = std::to_string(*(UA_Float*)value.data); r.type = "Float";
        } else if (value.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
            r.value = std::to_string(*(UA_Double*)value.data); r.type = "Double";
        } else if (value.type == &UA_TYPES[UA_TYPES_STRING]) {
            r.value = uaStringToStd(*(UA_String*)value.data); r.type = "String";
        } else {
            r.value = "<unsupported>"; r.type = "Other";
        }
    }

    UA_Variant_clear(&value);
    UA_NodeId_clear(&nid);
#endif
    return r;
}

bool Open62541Client::writeValue(const std::string& nodeId, const std::string& value) {
#ifdef WITH_OPEN62541
    if (!m_connected || !m_client) return false;
    UA_NodeId nid;
    UA_String nodeIdStr = UA_STRING_ALLOC(nodeId.c_str());
    UA_NodeId_parse(&nid, nodeIdStr);
    UA_String_clear(&nodeIdStr);

    UA_Variant outValue;
    UA_Variant_init(&outValue);
    UA_StatusCode ret = UA_Client_readValueAttribute(m_client, nid, &outValue);
    
    UA_Variant v;
    UA_Variant_init(&v);

    if (ret == UA_STATUSCODE_GOOD) {
        try {
            if (outValue.type == &UA_TYPES[UA_TYPES_DOUBLE]) {
                double d = std::stod(value);
                UA_Variant_setScalarCopy(&v, &d, &UA_TYPES[UA_TYPES_DOUBLE]);
            } else if (outValue.type == &UA_TYPES[UA_TYPES_FLOAT]) {
                float f = std::stof(value);
                UA_Variant_setScalarCopy(&v, &f, &UA_TYPES[UA_TYPES_FLOAT]);
            } else if (outValue.type == &UA_TYPES[UA_TYPES_INT32]) {
                int32_t i = std::stoi(value);
                UA_Variant_setScalarCopy(&v, &i, &UA_TYPES[UA_TYPES_INT32]);
            } else if (outValue.type == &UA_TYPES[UA_TYPES_INT64]) {
                long long i = std::stoll(value);
                UA_Variant_setScalarCopy(&v, &i, &UA_TYPES[UA_TYPES_INT64]);
            } else if (outValue.type == &UA_TYPES[UA_TYPES_BOOLEAN]) {
                UA_Boolean b = (value == "true" || value == "1");
                UA_Variant_setScalarCopy(&v, &b, &UA_TYPES[UA_TYPES_BOOLEAN]);
            } else {
                UA_String s = UA_STRING_ALLOC(value.c_str());
                UA_Variant_setScalarCopy(&v, &s, &UA_TYPES[UA_TYPES_STRING]);
                UA_String_clear(&s);
            }
        } catch (...) {
            UA_NodeId_clear(&nid);
            UA_Variant_clear(&outValue);
            return false;
        }
    }

    ret = UA_Client_writeValueAttribute(m_client, nid, &v);
    UA_Variant_clear(&outValue);
    UA_Variant_clear(&v);
    UA_NodeId_clear(&nid);
    return ret == UA_STATUSCODE_GOOD;
#else
    return false;
#endif
}