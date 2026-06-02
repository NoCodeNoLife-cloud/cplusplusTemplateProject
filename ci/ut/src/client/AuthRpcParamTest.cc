/**
 * @file AuthRpcParamTest.cc
 * @brief Unit tests for the AuthRpcParam class
 * @details Tests cover construction, getter/setter, Builder pattern,
 *          and default values for gRPC authentication parameters.
 */

#include <gtest/gtest.h>

#include "auth/AuthRpcParam.hpp"

using namespace client_app::auth;

/**
 * @brief Test fixture for AuthRpcParam tests
 */
class AuthRpcParamTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/**
 * @brief Test default construction of AuthRpcParam
 * @details Verifies that a default-constructed object has expected default values
 */
TEST_F(AuthRpcParamTest, DefaultConstruction)
{
    const AuthRpcParam param;

    EXPECT_EQ(param.keepaliveTimeMs(), 30 * 1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5 * 1000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
    EXPECT_EQ(param.serverAddress(), "localhost:50051");
}

/**
 * @brief Test custom construction of AuthRpcParam
 * @details Verifies that all parameters can be set via constructor
 */
TEST_F(AuthRpcParamTest, CustomConstruction)
{
    const AuthRpcParam param(10000, 3000, 0, "192.168.1.1:8080");

    EXPECT_EQ(param.keepaliveTimeMs(), 10000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 3000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "192.168.1.1:8080");
}

/**
 * @brief Test getter and setter for keepaliveTimeMs
 * @details Verifies that keepalive time can be read and written
 */
TEST_F(AuthRpcParamTest, KeepaliveTimeMsSetter)
{
    AuthRpcParam param;

    param.keepaliveTimeMs(15000);
    EXPECT_EQ(param.keepaliveTimeMs(), 15000);

    param.keepaliveTimeMs(0);
    EXPECT_EQ(param.keepaliveTimeMs(), 0);
}

/**
 * @brief Test getter and setter for keepaliveTimeoutMs
 * @details Verifies that keepalive timeout can be read and written
 */
TEST_F(AuthRpcParamTest, KeepaliveTimeoutMsSetter)
{
    AuthRpcParam param;

    param.keepaliveTimeoutMs(8000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 8000);

    param.keepaliveTimeoutMs(1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 1000);
}

/**
 * @brief Test getter and setter for keepalivePermitWithoutCalls
 * @details Verifies that permit flag can be toggled
 */
TEST_F(AuthRpcParamTest, KeepalivePermitWithoutCallsSetter)
{
    AuthRpcParam param;

    param.keepalivePermitWithoutCalls(0);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);

    param.keepalivePermitWithoutCalls(1);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
}

/**
 * @brief Test getter and setter for serverAddress
 * @details Verifies that server address can be read and written
 */
TEST_F(AuthRpcParamTest, ServerAddressSetter)
{
    AuthRpcParam param;

    param.serverAddress("10.0.0.1:50051");
    EXPECT_EQ(param.serverAddress(), "10.0.0.1:50051");

    param.serverAddress("example.com:443");
    EXPECT_EQ(param.serverAddress(), "example.com:443");
}

/**
 * @brief Test Builder with no overrides produces default values
 * @details Verifies that Builder::build() with no method calls
 *          returns an object with all default values
 */
TEST_F(AuthRpcParamTest, BuilderDefaultValues)
{
    const auto param = AuthRpcParam::builder().build();

    EXPECT_EQ(param.keepaliveTimeMs(), 30 * 1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5 * 1000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
    EXPECT_EQ(param.serverAddress(), "localhost:50051");
}

/**
 * @brief Test Builder with single field override
 * @details Verifies that setting only one field via Builder
 *          leaves other fields at their defaults
 */
TEST_F(AuthRpcParamTest, BuilderSingleField)
{
    const auto param = AuthRpcParam::builder()
        .serverAddress("10.0.0.1:50051")
        .build();

    EXPECT_EQ(param.serverAddress(), "10.0.0.1:50051");
    EXPECT_EQ(param.keepaliveTimeMs(), 30 * 1000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5 * 1000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 1);
}

/**
 * @brief Test Builder with all fields overridden
 * @details Verifies that all fields can be set via Builder
 */
TEST_F(AuthRpcParamTest, BuilderAllFields)
{
    const auto param = AuthRpcParam::builder()
        .keepaliveTimeMs(10000)
        .keepaliveTimeoutMs(3000)
        .keepalivePermitWithoutCalls(0)
        .serverAddress("192.168.1.1:8080")
        .build();

    EXPECT_EQ(param.keepaliveTimeMs(), 10000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 3000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "192.168.1.1:8080");
}

/**
 * @brief Test Builder method chaining returns Builder reference
 * @details Verifies that each builder method returns *this
 *          to enable method chaining
 */
TEST_F(AuthRpcParamTest, BuilderChaining)
{
    auto& ref1 = AuthRpcParam::builder().keepaliveTimeMs(10000);
    auto& ref2 = ref1.keepaliveTimeoutMs(5000);
    auto& ref3 = ref2.keepalivePermitWithoutCalls(0);
    auto& ref4 = ref3.serverAddress("127.0.0.1:50051");

    const auto param = ref4.build();
    EXPECT_EQ(param.keepaliveTimeMs(), 10000);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 5000);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), 0);
    EXPECT_EQ(param.serverAddress(), "127.0.0.1:50051");
}

/**
 * @brief Test Builder produces independent objects
 * @details Verifies that multiple build() calls produce
 *          independent AuthRpcParam instances
 */
TEST_F(AuthRpcParamTest, BuilderIndependentBuilds)
{
    auto builder = AuthRpcParam::builder()
        .keepaliveTimeMs(10000);

    const auto param1 = builder.build();
    builder.keepaliveTimeMs(20000);
    const auto param2 = builder.build();

    EXPECT_EQ(param1.keepaliveTimeMs(), 10000);
    EXPECT_EQ(param2.keepaliveTimeMs(), 20000);
}

/**
 * @brief Test Builder with extreme values
 * @details Verifies that Builder can handle boundary values
 */
TEST_F(AuthRpcParamTest, BuilderExtremeValues)
{
    const auto param = AuthRpcParam::builder()
        .keepaliveTimeMs(0)
        .keepaliveTimeoutMs(0)
        .keepalivePermitWithoutCalls(-1)
        .serverAddress("")
        .build();

    EXPECT_EQ(param.keepaliveTimeMs(), 0);
    EXPECT_EQ(param.keepaliveTimeoutMs(), 0);
    EXPECT_EQ(param.keepalivePermitWithoutCalls(), -1);
    EXPECT_EQ(param.serverAddress(), "");
}

/**
 * @brief Test copy construction is allowed
 * @details Verifies that AuthRpcParam can be copy-constructed
 *          Copy assignment is deleted to prevent unintended resource duplication
 */
TEST_F(AuthRpcParamTest, CopyableNotAssignable)
{
    EXPECT_TRUE(std::is_copy_constructible_v<AuthRpcParam>);
    EXPECT_FALSE(std::is_copy_assignable_v<AuthRpcParam>);
}
