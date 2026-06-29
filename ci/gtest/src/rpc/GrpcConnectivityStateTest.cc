/**
 * @file GrpcConnectivityStateTest.cc
 * @brief Unit tests for the GrpcConnectivityState enum
 * @details Tests cover enum value existence, magic_enum integration,
 *          comparison operators, and switch exhaustiveness.
 */

#include <algorithm>
#include <ranges>
#include <string>
#include <gtest/gtest.h>
#include <magic_enum/magic_enum.hpp>

#include <cppforge/rpc/GrpcConnectivityState.hpp>

using namespace cppforge::rpc;

/**
 * @brief Test fixture for GrpcConnectivityState tests
 */
class GrpcConnectivityStateTest : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @brief Test enum has all five expected values
 */
TEST_F(GrpcConnectivityStateTest, EnumValueCount)
{
    EXPECT_EQ(magic_enum::enum_count<GrpcConnectivityState>(), 5);
}

/**
 * @brief Test all enum values exist in the values list
 */
TEST_F(GrpcConnectivityStateTest, EnumValuesContainAll)
{
    constexpr auto values = magic_enum::enum_values<GrpcConnectivityState>();
    EXPECT_EQ(values.size(), 5);
    EXPECT_NE(std::ranges::find(values, GrpcConnectivityState::IDLE), values.end());
    EXPECT_NE(std::ranges::find(values, GrpcConnectivityState::CONNECTING), values.end());
    EXPECT_NE(std::ranges::find(values, GrpcConnectivityState::READY), values.end());
    EXPECT_NE(std::ranges::find(values, GrpcConnectivityState::TRANSIENT_FAILURE), values.end());
    EXPECT_NE(std::ranges::find(values, GrpcConnectivityState::SHUTDOWN), values.end());
}

/**
 * @brief Test enum name conversion via magic_enum
 */
TEST_F(GrpcConnectivityStateTest, EnumNames)
{
    EXPECT_EQ(magic_enum::enum_name(GrpcConnectivityState::IDLE), "IDLE");
    EXPECT_EQ(magic_enum::enum_name(GrpcConnectivityState::CONNECTING), "CONNECTING");
    EXPECT_EQ(magic_enum::enum_name(GrpcConnectivityState::READY), "READY");
    EXPECT_EQ(magic_enum::enum_name(GrpcConnectivityState::TRANSIENT_FAILURE), "TRANSIENT_FAILURE");
    EXPECT_EQ(magic_enum::enum_name(GrpcConnectivityState::SHUTDOWN), "SHUTDOWN");
}

/**
 * @brief Test name-to-enum roundtrip via magic_enum
 */
TEST_F(GrpcConnectivityStateTest, EnumRoundtrip)
{
    for (const auto state : magic_enum::enum_values<GrpcConnectivityState>())
    {
        const auto name = magic_enum::enum_name(state);
        const auto parsed = magic_enum::enum_cast<GrpcConnectivityState>(name);
        ASSERT_TRUE(parsed.has_value());
        EXPECT_EQ(parsed.value(), state);
    }
}

/**
 * @brief Test enum integer conversion via magic_enum
 */
TEST_F(GrpcConnectivityStateTest, EnumIntegerConversion)
{
    EXPECT_EQ(magic_enum::enum_integer(GrpcConnectivityState::IDLE), 0);
    EXPECT_EQ(magic_enum::enum_integer(GrpcConnectivityState::CONNECTING), 1);
    EXPECT_EQ(magic_enum::enum_integer(GrpcConnectivityState::READY), 2);
    EXPECT_EQ(magic_enum::enum_integer(GrpcConnectivityState::TRANSIENT_FAILURE), 3);
    EXPECT_EQ(magic_enum::enum_integer(GrpcConnectivityState::SHUTDOWN), 4);
}

/**
 * @brief Test enum comparison operators
 */
TEST_F(GrpcConnectivityStateTest, EnumComparison)
{
    EXPECT_EQ(GrpcConnectivityState::IDLE, GrpcConnectivityState::IDLE);
    EXPECT_NE(GrpcConnectivityState::IDLE, GrpcConnectivityState::READY);
    EXPECT_LT(GrpcConnectivityState::IDLE, GrpcConnectivityState::CONNECTING);
    EXPECT_LE(GrpcConnectivityState::IDLE, GrpcConnectivityState::IDLE);
    EXPECT_GT(GrpcConnectivityState::SHUTDOWN, GrpcConnectivityState::READY);
    EXPECT_GE(GrpcConnectivityState::SHUTDOWN, GrpcConnectivityState::SHUTDOWN);
}

/**
 * @brief Test switch exhaustiveness
 * @details Verifies all five enum values can be switched over.
 *          Missing a value here triggers a compiler warning.
 */
TEST_F(GrpcConnectivityStateTest, SwitchExhaustiveness)
{
    int count = 0;
    for (const auto state : magic_enum::enum_values<GrpcConnectivityState>())
    {
        switch (state)
        {
            case GrpcConnectivityState::IDLE:
            case GrpcConnectivityState::CONNECTING:
            case GrpcConnectivityState::READY:
            case GrpcConnectivityState::TRANSIENT_FAILURE:
            case GrpcConnectivityState::SHUTDOWN:
                count++;
                break;
            default:
                break;
        }
    }
    EXPECT_EQ(count, 5);
}

/**
 * @brief Test all enum names are non-empty
 */
TEST_F(GrpcConnectivityStateTest, AllNamesNonEmpty)
{
    for (const auto state : magic_enum::enum_values<GrpcConnectivityState>())
    {
        const auto name = magic_enum::enum_name(state);
        EXPECT_FALSE(name.empty());
    }
}
