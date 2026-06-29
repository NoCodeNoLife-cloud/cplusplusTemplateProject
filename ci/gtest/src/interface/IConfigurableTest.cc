/**
 * @file IConfigurableTest.cc
 * @brief Unit tests for IConfigurable interface
 * @details Tests cover the Template Method pattern where config() delegates to doConfig().
 */

#include <gtest/gtest.h>

#include <cppforge/interface/IConfigurable.hpp>

using namespace cppforge::interface::config;

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

/// @brief Test fixture for IConfigurable tests.
class IConfigurableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        configurable_ = std::make_unique<MockConfigurable>();
    }

    void TearDown() override
    {
        configurable_.reset();
    }

    std::unique_ptr<MockConfigurable> configurable_;
};

/** @brief Verifies config() returns true when doConfig succeeds. */
TEST_F(IConfigurableTest, ConfigReturnsTrueOnSuccess)
{
    const bool result = configurable_->config();

    EXPECT_TRUE(result);
    EXPECT_EQ(configurable_->doConfigCallCount_, 1);
}

/** @brief Verifies config() delegates to doConfig() internally. */
TEST_F(IConfigurableTest, ConfigDelegatesToDoConfig)
{
    (void)configurable_->config();
    (void)configurable_->config();

    EXPECT_EQ(configurable_->doConfigCallCount_, 2);
}

/** @brief Verifies config() returns false when doConfig fails. */
TEST_F(IConfigurableTest, ConfigReturnsFalseOnDoConfigFailure)
{
    configurable_->returnValue_ = false;

    const bool result = configurable_->config();

    EXPECT_FALSE(result);
    EXPECT_EQ(configurable_->doConfigCallCount_, 1);
}

/** @brief Verifies config() can be invoked multiple times. */
TEST_F(IConfigurableTest, ConfigCanBeCalledMultipleTimes)
{
    EXPECT_TRUE(configurable_->config());
    EXPECT_TRUE(configurable_->config());
    EXPECT_TRUE(configurable_->config());

    EXPECT_EQ(configurable_->doConfigCallCount_, 3);
}

/** @brief Verifies config() does not throw under normal conditions. */
TEST_F(IConfigurableTest, ConfigDoesNotThrowByDefault)
{
    EXPECT_NO_THROW((void)configurable_->config());
}

/** @brief Verifies the return value of config() can be tested. */
TEST_F(IConfigurableTest, ConfigIsMarkedNoDiscard)
{
    const auto result = configurable_->config();
    EXPECT_TRUE(result);
}
