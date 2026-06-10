/**
 * @file ConfigParamTest.cc
 * @brief Unit tests for the client ConfigParam singleton
 * @details Tests cover singleton access and application dev config path behavior.
 */

#include <gtest/gtest.h>

#include "config/ConfigParam.h"

using namespace client_app::config;

class ClientConfigParamTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(ClientConfigParamTest, GetInstance_ReturnsSameInstance)
{
    auto& instance1 = ConfigParam::getInstance();
    auto& instance2 = ConfigParam::getInstance();
    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(ClientConfigParamTest, ApplicationDevConfigPath_ThrowsWhenMissing)
{
    auto& instance = ConfigParam::getInstance();
    EXPECT_THROW(instance.applicationDevConfigPath(), std::runtime_error);
}
