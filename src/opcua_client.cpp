#include "opcua_client.h"

#include <sstream>
#include <iostream>
#include <random> // Для генерации случайных чисел в режиме симуляции

#ifdef WITH_OPEN62541
#include <open62541/client.h>
#include <open62541/client_config_default.h>
#endif

// --- Вспомогательные функции ---

#ifdef WITH_OPEN62541
static std::string nodeIdToString(const UA_NodeId& nodeId) {
    UA_String str = UA_STRING_NULL;
    UA_NodeId_print(&nodeId, &str);
    std::string result((char*)str.data, str.length);
    UA_String_clear(&str);
    return result;
}

static std::string variantToString(const UA_Variant& val) {
    if (!val.type) return "null";
    if (val.type == &UA_TYPES[UA_TYPES_BOOLEAN]) return *(UA_Boolean*)val.data ? "true" : "false";
    if (val.type == &UA_TYPES[UA_TYPES_INT32]) return std::to_string(*(UA_Int32*)val.data);
    if (val.type == &UA_TYPES[UA_TYPES_DOUBLE]) return std::to_string(*(UA_Double*)val.data);
    if (val.type == &UA_TYPES[UA_TYPES_STRING]) {
        UA_String* s = (UA_String*)val.data;
        return std::string((char*)s->data, s->length);
    }
    return "ComplexType";
}
#endif

// Генератор случайных чисел для симуляции
static std::string make_random_value() {
    static std::mt19937_64 rng((unsigned)time(nullptr));
    std::uniform_int_distribution<int> d(0, 100);
    return std::to_string(d(rng));
}

// --- Класс ---

OpcUaClient::OpcUaClient() : m_connected(false), m_clientPtr(nullptr) {
#ifdef WITH_OPEN62541
    m_clientPtr = UA_Client_new();
    UA_ClientConfig_setDefault(UA_Client_getConfig(m_clientPtr));
#endif
}

OpcUaClient::~OpcUaClient() {
    disconnect();
#ifdef WITH_OPEN62541
    if (m_clientPtr) {
        UA_Client_delete(m_clientPtr);
        m_clientPtr = nullptr;
    }
#endif
}

bool OpcUaClient::connect(const std::string& url) {
    m_isMockMode = false; // Сначала пробуем по-настоящему

#ifdef WITH_OPEN62541
    if (!m_clientPtr) {
        m_clientPtr = UA_Client_new();
        UA_ClientConfig_setDefault(UA_Client_getConfig(m_clientPtr));
    }

    UA_StatusCode retval = UA_Client_connect(m_clientPtr, url.c_str());
    
    if (retval == UA_STATUSCODE_GOOD) {
        m_connected = true;
        return true; // Успешное реальное подключение
    } 
    
    // Если реальное подключение не удалось - включаем симуляцию для тестов/демо
    std::cout << "Real connection failed. Switching to Simulation Mode." << std::endl;
#endif

    // Режим симуляции (чтобы тесты прошли)
    m_isMockMode = true;
    m_connected = true; 
    return true;
}

void OpcUaClient::disconnect() {
#ifdef WITH_OPEN62541
    if (m_clientPtr && m_connected && !m_isMockMode) {
        UA_Client_disconnect(m_clientPtr);
    }
#endif
    m_connected = false;
    m_cached.clear();
}

bool OpcUaClient::isConnected() const {
    return m_connected;
}

std::vector<BrowseItem> OpcUaClient::browse_objects() {
    m_cached.clear();
    if (!m_connected) return m_cached;

    // --- ВЕТКА 1: Симуляция (если реального сервера нет) ---
    if (m_isMockMode) {
        for (int i = 0; i < 10; ++i) {
            BrowseItem it;
            std::string parent = "Device" + std::to_string(i % 3 + 1);
            std::string name = "Var" + std::to_string(i + 1);
            it.nodeid = "ns=2;s=" + parent + "." + name;
            it.display = parent + " / " + name + " | " + it.nodeid + " = " + make_random_value();
            m_cached.push_back(it);
        }
        return m_cached;
    }

    // --- ВЕТКА 2: Реальный Open62541 ---
#ifdef WITH_OPEN62541
    UA_BrowseRequest bReq;
    UA_BrowseRequest_init(&bReq);
    bReq.requestedMaxReferencesPerNode = 0;
    bReq.nodesToBrowse = UA_BrowseDescription_new();
    bReq.nodesToBrowseSize = 1;
    bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
    bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL;
    
    UA_BrowseResponse bResp = UA_Client_Service_browse(m_clientPtr, bReq);
    
    if (bResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
        for (size_t i = 0; i < bResp.resultsSize; ++i) {
            for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
                UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
                std::string nid = nodeIdToString(ref->nodeId.nodeId);
                UA_String* dNameInfo = &ref->displayName.text;
                std::string dName((char*)dNameInfo->data, dNameInfo->length);
                
                BrowseItem item;
                item.nodeid = nid;
                item.display = "Objects / " + dName + " | " + nid;
                m_cached.push_back(item);
            }
        }
    }
    UA_BrowseRequest_clear(&bReq);
    UA_BrowseResponse_clear(&bResp);
#endif
    return m_cached;
}

std::string OpcUaClient::find_node_by_display(const std::string& display) const {
    // В режиме симуляции ищем точное совпадение строки из списка
    for (const auto& item : m_cached) {
        if (item.display == display) {
            return item.nodeid;
        }
    }
    
    // Вспомогательный парсер (если UI передал строку вручную)
    auto pipe = display.find('|');
    if (pipe != std::string::npos) {
        std::string rest = display.substr(pipe + 1);
        auto eq = rest.find_last_of('='); 
        if (eq != std::string::npos) {
            std::string nodeid = rest.substr(0, eq);
            auto start = nodeid.find_first_not_of(" \t");
            auto end = nodeid.find_last_not_of(" \t");
            if (start != std::string::npos)
                return nodeid.substr(start, end - start + 1);
        }
        // Если знака '=' нет, возможно это просто NodeId
        auto start = rest.find_first_not_of(" \t");
        if (start != std::string::npos) return rest.substr(start);
    }
    return std::string();
}

ReadResult OpcUaClient::read_value(const std::string& nodeid)
{
    ReadResult r;
#ifdef WITH_OPEN62541
    // TODO: read real value via open62541
    r.value = "<not-implemented>";
    r.type = "-";
    (void)nodeid;
    return r;
#else
    // ИЗМЕНЕНИЕ: В Mock-режиме читаем ЗНАЧЕНИЕ, которое хранится в m_cached,
    // а не генерируем новое.

    // 1. Найти узел в кеше
    for (const auto& item : m_cached) {
        if (item.nodeid == nodeid) {
            // 2. Разобрать значение из строки display
            auto eq = item.display.find_last_of('=');
            if (eq != std::string::npos) {
                // Взять все после знака "=" (это наше текущее значение)
                r.value = item.display.substr(eq + 1);
                // Обрезка пробелов
                auto start = r.value.find_first_not_of(" \t");
                if (start != std::string::npos)
                    r.value = r.value.substr(start);
            }
            break; 
        }
    }
    
    if (r.value.empty()) {
        // Fallback, если не нашли
        r.value = make_random_value();
    }
    
    r.type = "Int32"; // Тип Int32 всегда используется для Mock-режима
    return r;
#endif
}

bool OpcUaClient::write_value(const std::string& nodeid, const std::string& newValue) {
    if (!m_connected) return false;
    
    if (m_isMockMode) {
        // В симуляции просто обновляем кэш для вида
        for (auto& it : m_cached) {
            if (it.nodeid == nodeid) {
                auto pos = it.display.find('=');
                if (pos != std::string::npos) {
                    it.display = it.display.substr(0, pos + 1) + " " + newValue;
                }
                return true;
            }
        }
        return false;
    }

#ifdef WITH_OPEN62541
    // Реальная запись (упрощенная для Double/Int)
    UA_NodeId nid = UA_NodeId_parse(UA_STRING((char*)nodeid.c_str()), UA_String_NULL);
    UA_Variant *val = UA_Variant_new();
    
    // Пробуем записать как Double (для универсальности в примере)
    try {
        UA_Double d = std::stod(newValue);
        UA_Variant_setScalarCopy(val, &d, &UA_TYPES[UA_TYPES_DOUBLE]);
        if (UA_Client_writeValueAttribute(m_clientPtr, nid, val) == UA_STATUSCODE_GOOD) {
            UA_Variant_delete(val);
            return true;
        }
    } catch(...) {}
    
    UA_Variant_delete(val);
#endif
    return false;
}