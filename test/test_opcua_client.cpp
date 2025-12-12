// Unit tests for OpcUaClient (skeleton mode)
#include "opcua_client.h"
#include <gtest/gtest.h>

TEST(OpcUaClientTest, ConnectDisconnect)
{
    OpcUaClient c;
    EXPECT_FALSE(c.isConnected());
    EXPECT_TRUE(c.connect("opc.tcp://localhost:4840"));
    EXPECT_TRUE(c.isConnected());
    c.disconnect();
    EXPECT_FALSE(c.isConnected());
}

TEST(OpcUaClientTest, BrowseReturnsItems)
{
    OpcUaClient c;
    c.connect("opc.tcp://localhost:4840");
    auto items = c.browse_objects();
    EXPECT_EQ(items.size(), 10u);
    EXPECT_FALSE(items.front().display.empty());
    EXPECT_FALSE(items.front().nodeid.empty());
    // find_node_by_display should return a node id for the item
    auto nid = c.find_node_by_display(items.front().display);
    EXPECT_FALSE(nid.empty());
}

TEST(OpcUaClientTest, ReadAndWrite)
{
    OpcUaClient c;
    c.connect("opc.tcp://localhost:4840");
    auto items = c.browse_objects();
    auto nid = c.find_node_by_display(items.front().display);
    ASSERT_FALSE(nid.empty());

    auto r = c.read_value(nid);
    EXPECT_FALSE(r.value.empty());
    EXPECT_FALSE(r.type.empty());

    bool ok = c.write_value(nid, "123");
    EXPECT_TRUE(ok);
}

TEST(OpcUaClientTest, ParseNodeIdFallback)
{
    OpcUaClient c;
    std::string display = "Device1 / Var1 | ns=2;s=Device1.Var1 = 42";
    auto nid = c.find_node_by_display(display);
    EXPECT_EQ(nid, std::string("ns=2;s=Device1.Var1"));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
