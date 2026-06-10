/**
 * @file ConfigParamTest.cc
 * @brief Unit tests for the server ConfigParam singleton
 * @details Tests cover singleton access and application dev config path behavior.
 */

#include <gtest/gtest.h>

#include "../../server/src/config/ConfigParam.hpp"

using namespace server_app::config;

class ServerConfigParamTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(ServerConfigParamTest, GetInstance_ReturnsSameInstance)
{
    auto& instance1 = ConfigParam::getInstance();
    auto& instance2 = ConfigParam::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(ServerConfigParamTest, ApplicationDevConfigPath_ThrowsWhenMissing)
{
    auto& instance = ConfigParam::getInstance();
    EXPECT_THROW(instance.applicationDevConfigPath(), std::runtime_error);
}
