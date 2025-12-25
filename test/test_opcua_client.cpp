// Unit tests for OpcUaClient using Google Test framework
#include "OpcUaClient.h"
#include <gtest/gtest.h>

// Test 1: OpcUaClient initialization - should start disconnected
TEST(OpcUaClientTest, InitialStateNotConnected)
{
    OpcUaClient client;
    EXPECT_FALSE(client.isConnected());
}

// Test 2: Browse objects returns valid items (mock mode generates 10 items)
TEST(OpcUaClientTest, BrowseObjectsReturnsData)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");  // Mock mode: always succeeds
    
    auto items = client.browse_objects();
    EXPECT_EQ(items.size(), 10u);
    
    // Each item should have valid nodeId and displayPath
    for (const auto& item : items) {
        EXPECT_FALSE(item.nodeId.empty());
        EXPECT_FALSE(item.displayPath.empty());
    }
}

// Test 3: Read value returns valid result structure
TEST(OpcUaClientTest, ReadValueReturnsValidData)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    auto result = client.read_value("ns=2;i=5");
    
    // Mock mode returns valid data (not an error marker)
    EXPECT_FALSE(result.value.empty());
    EXPECT_FALSE(result.type.empty());
    EXPECT_NE(result.type, "<error>");
}

// Test 4: Write value succeeds in mock mode
TEST(OpcUaClientTest, WriteValueSucceeds)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    bool success = client.write_value("ns=2;i=5", "42.5");
    EXPECT_TRUE(success);
}

// Test 5: Connect and disconnect state transitions
TEST(OpcUaClientTest, ConnectDisconnectCycle)
{
    OpcUaClient client;
    
    // Initially disconnected
    EXPECT_FALSE(client.isConnected());
    
    // After connect
    bool connected = client.connect("opc.tcp://localhost:4840");
    EXPECT_TRUE(connected);
    EXPECT_TRUE(client.isConnected());
    
    // After disconnect
    client.disconnect();
    EXPECT_FALSE(client.isConnected());
}

// Test 6: Browse returns consistent data on repeated calls
TEST(OpcUaClientTest, BrowseDataConsistency)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    auto items1 = client.browse_objects();
    auto items2 = client.browse_objects();
    
    // Mock mode returns identical data each time
    EXPECT_EQ(items1.size(), items2.size());
    if (!items1.empty() && !items2.empty()) {
        EXPECT_EQ(items1.front().nodeId, items2.front().nodeId);
    }
}

// Test 7: Multiple reads of same node return results
TEST(OpcUaClientTest, MultipleReadsSucceed)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    auto result1 = client.read_value("ns=2;i=1");
    auto result2 = client.read_value("ns=2;i=2");
    
    EXPECT_FALSE(result1.value.empty());
    EXPECT_FALSE(result2.value.empty());
}

// Test 8: Browse then read from first item works
TEST(OpcUaClientTest, BrowseThenRead)
{
    OpcUaClient client;
    client.connect("opc.tcp://localhost:4840");
    
    auto items = client.browse_objects();
    ASSERT_EQ(items.size(), 10u);
    
    // Read value from first item using its nodeId
    auto result = client.read_value(items[0].nodeId);
    EXPECT_FALSE(result.value.empty());
    EXPECT_FALSE(result.type.empty());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
