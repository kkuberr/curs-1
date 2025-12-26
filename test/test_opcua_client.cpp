#include "OpcUaClient.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(OpcUaClientTest, InitialStateNotConnected)
{
    OpcUaClient client;
    EXPECT_FALSE(client.isConnected());
}

TEST(OpcUaClientTest, BrowseObjectsReturnsData)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");  
    
    auto items = client.browse_objects();
    EXPECT_EQ(items.size(), 10u);
    
    for (const auto& item : items) {
        EXPECT_FALSE(item.nodeId.empty());
        EXPECT_FALSE(item.displayPath.empty());
    }
}

TEST(OpcUaClientTest, ReadValueReturnsValidData)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    auto result = client.read_value("ns=2;i=5");
    
    EXPECT_FALSE(result.value.empty());
    EXPECT_FALSE(result.type.empty());
    EXPECT_NE(result.type, "<error>");
}

TEST(OpcUaClientTest, WriteValueSucceeds)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    bool success = client.write_value("ns=2;i=5", "42.5");
    EXPECT_TRUE(success);
}

TEST(OpcUaClientTest, ConnectDisconnectCycle)
{
    OpcUaClient client;
    EXPECT_FALSE(client.isConnected());
    
    bool connected = client.connect("opc.tcp://localhost:4840");
    EXPECT_TRUE(connected);
    EXPECT_TRUE(client.isConnected());
    
    client.disconnect();
    EXPECT_FALSE(client.isConnected());
}

TEST(OpcUaClientTest, BrowseDataConsistency)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    auto items1 = client.browse_objects();
    auto items2 = client.browse_objects();
    
    EXPECT_EQ(items1.size(), items2.size());
    if (!items1.empty() && !items2.empty()) {
        EXPECT_EQ(items1.front().nodeId, items2.front().nodeId);
    }
}

TEST(OpcUaClientTest, MultipleReadsSucceed)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    auto result1 = client.read_value("ns=2;i=1");
    auto result2 = client.read_value("ns=2;i=2");
    
    EXPECT_FALSE(result1.value.empty());
    EXPECT_FALSE(result2.value.empty());
}

TEST(OpcUaClientTest, BrowseThenRead)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    auto items = client.browse_objects();
    ASSERT_EQ(items.size(), 10u);
    
    auto result = client.read_value(items[0].nodeId);
    EXPECT_FALSE(result.value.empty());
    EXPECT_FALSE(result.type.empty());
}

TEST(OpcUaClientTest, RealServerIntegrationTest)
{
    OpcUaClient client;
    std::string realUrl = "opc.tcp://127.0.0.1:4840/freeopcua/server/";
    
    std::cout << "[ INFO ] Попытка подключения к реальному серверу: " << realUrl << std::endl;
    
    bool connected = client.connect(realUrl);
    
    if (connected) {
        EXPECT_TRUE(client.isConnected());
        
        auto items = client.browse_objects();
        std::cout << "[ OK ] Соединение установлено. Найдено узлов: " << items.size() << std::endl;
        
        if (!items.empty()) {
            auto val = client.read_value(items[0].nodeId);
            EXPECT_FALSE(val.value.empty());
            std::cout << "[ OK ] Значение первого узла прочитано: " << val.value << std::endl;
        }
        
        client.disconnect();
    } else {
        std::cout << "[ SKIP ] Реальный сервер не доступен по адресу " << realUrl << std::endl;
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}