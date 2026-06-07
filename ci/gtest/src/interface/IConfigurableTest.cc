/**
 * @file IConfigurableTest.cc
 * @brief Unit tests for IConfigurable interface
 * @details Tests cover the Template Method pattern where config() delegates to doConfig().
 */

#include <gtest/gtest.h>

#include "interface/config/IConfigurable.hpp"

using namespace common::interface::config;

namespace
{
    /// @brief Mock configurable that tracks configuration calls
    class MockConfigurable : public IConfigurable
    {
    public:
        int doConfigCallCount_ = 0;
        bool returnValue_ = true;

    protected:
        bool doConfig() override
        {
            doConfigCallCount_++;
            return returnValue_;
        }
    };
}

class IConfigurableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        configurable_.reset(new MockConfigurable());
    }

    void TearDown() override
    {
        configurable_.reset();
    }

    std::unique_ptr<MockConfigurable> configurable_;
};

TEST_F(IConfigurableTest, ConfigReturnsTrueOnSuccess)
{
    bool result = configurable_->config();

    EXPECT_TRUE(result);
    EXPECT_EQ(configurable_->doConfigCallCount_, 1);
}

TEST_F(IConfigurableTest, ConfigDelegatesToDoConfig)
{
    configurable_->config();
    configurable_->config();

    EXPECT_EQ(configurable_->doConfigCallCount_, 2);
}

TEST_F(IConfigurableTest, ConfigReturnsFalseOnDoConfigFailure)
{
    configurable_->returnValue_ = false;

    bool result = configurable_->config();

    EXPECT_FALSE(result);
    EXPECT_EQ(configurable_->doConfigCallCount_, 1);
}

TEST_F(IConfigurableTest, ConfigCanBeCalledMultipleTimes)
{
    EXPECT_TRUE(configurable_->config());
    EXPECT_TRUE(configurable_->config());
    EXPECT_TRUE(configurable_->config());

    EXPECT_EQ(configurable_->doConfigCallCount_, 3);
}

TEST_F(IConfigurableTest, ConfigDoesNotThrowByDefault)
{
    EXPECT_NO_THROW(configurable_->config());
}

TEST_F(IConfigurableTest, ConfigIsMarkedNoDiscard)
{
    auto result = configurable_->config();
    EXPECT_TRUE(result);
}
