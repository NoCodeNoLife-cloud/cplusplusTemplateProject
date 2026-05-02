/**
 * @file StaticObjectWrapperTest.cc
 * @brief Unit tests for the StaticObjectWrapper class
 * @details Tests cover initialization, retrieval, destruction, and state management.
 */

#include <gtest/gtest.h>
#include "toolkit/StaticObjectWrapper.hpp"
#include <string>
#include <type_traits>

using namespace common::toolkit;

/**
 * @brief Simple test class for wrapper tests
 */
struct TestConfig {
    std::string name;
    int value;

    TestConfig() : name("default"), value(0) {
    }

    TestConfig(const std::string &n, int v) : name(n), value(v) {
    }
};

// Non-default constructible class
struct NoDefaultConfig {
    std::string name;
    int value;

    NoDefaultConfig(const std::string &n, int v) : name(n), value(v) {
    }
};

/**
 * @brief Wrapper classes to create distinct types for test isolation
 */
class TestConfigWrapper1 {
};

class TestConfigWrapper2 {
};

class TestConfigWrapper3 {
};

class TestConfigWrapper6 {
};

class TestConfigWrapper7 {
};

class TestConfigWrapper8 {
};

class NoDefaultConfigWrapper4 {
};

class NoDefaultConfigWrapper5 {
};

/**
 * @brief Type aliases with unique wrapper types to isolate static state between tests
 */
using Wrapper1 = StaticObjectWrapper<TestConfig>;
using Wrapper2 = StaticObjectWrapper<std::pair<TestConfig, TestConfigWrapper2> >;
using Wrapper3 = StaticObjectWrapper<std::pair<TestConfig, TestConfigWrapper3> >;
using Wrapper4 = StaticObjectWrapper<NoDefaultConfig>;
using Wrapper5 = StaticObjectWrapper<std::pair<NoDefaultConfig, NoDefaultConfigWrapper5> >;
using Wrapper6 = StaticObjectWrapper<std::pair<TestConfig, TestConfigWrapper6> >;
using Wrapper7 = StaticObjectWrapper<std::pair<TestConfig, TestConfigWrapper7> >;
using Wrapper8 = StaticObjectWrapper<std::pair<TestConfig, TestConfigWrapper8> >;

/**
 * @brief Test init with parameters and get
 * @details Verifies parameterized initialization and retrieval
 */
TEST(StaticObjectWrapperTest, InitWithParamsAndGet) {
    // Clean state
    Wrapper1::destroy();
    EXPECT_FALSE(Wrapper1::isInitialized());

    // Initialize with parameters
    Wrapper1::init("test_config", 42);
    EXPECT_TRUE(Wrapper1::isInitialized());

    // Get and verify
    const auto &config = Wrapper1::get();
    EXPECT_EQ(config.name, "test_config");
    EXPECT_EQ(config.value, 42);

    // Cleanup
    Wrapper1::destroy();
}

// Test init without parameters (default constructible)
TEST(StaticObjectWrapperTest, InitDefaultConstructible) {
    // Clean state
    Wrapper2::destroy();

    // Initialize without parameters (should use default constructor)
    Wrapper2::init();
    EXPECT_TRUE(Wrapper2::isInitialized());

    // Get and verify default values
    const auto &config = Wrapper2::get();
    EXPECT_EQ(config.first.name, "default");
    EXPECT_EQ(config.first.value, 0);

    // Cleanup
    Wrapper2::destroy();
}

// Test get without initialization for default constructible type
TEST(StaticObjectWrapperTest, GetWithoutInitDefaultConstructible) {
    // Clean state
    Wrapper3::destroy();
    EXPECT_FALSE(Wrapper3::isInitialized());

    // Get should auto-initialize for default constructible types
    const auto &[fst, snd] = Wrapper3::get();
    EXPECT_TRUE(Wrapper3::isInitialized());
    EXPECT_EQ(fst.name, "default");
    EXPECT_EQ(fst.value, 0);

    // Cleanup
    Wrapper3::destroy();
}

// Test get without initialization for non-default constructible type
TEST(StaticObjectWrapperTest, GetWithoutInitNonDefaultConstructible) {
    // Clean state
    Wrapper4::destroy();
    EXPECT_FALSE(Wrapper4::isInitialized());

    // Get should throw for non-default constructible types without init
    EXPECT_THROW(Wrapper4::get(), std::runtime_error);
    EXPECT_FALSE(Wrapper4::isInitialized());
}

// Test init with non-default constructible type
TEST(StaticObjectWrapperTest, InitNonDefaultConstructible) {
    // Clean state
    Wrapper5::destroy();

    // Initialize with required parameters - construct pair explicitly
    const auto config_pair = std::make_pair(NoDefaultConfig{"custom_config", 100}, NoDefaultConfigWrapper5{});
    Wrapper5::init(config_pair);
    EXPECT_TRUE(Wrapper5::isInitialized());

    // Get and verify
    const auto &config = Wrapper5::get();
    EXPECT_EQ(config.first.name, "custom_config");
    EXPECT_EQ(config.first.value, 100);

    // Cleanup
    Wrapper5::destroy();
}

/**
 * @brief Test destroy functionality
 * @details Verifies object can be destroyed and state is reset
 */
TEST(StaticObjectWrapperTest, Destroy) {
    // Initialize with explicit pair construction
    const auto config_pair = std::make_pair(TestConfig{"test", 123}, TestConfigWrapper6{});
    Wrapper6::init(config_pair);
    EXPECT_TRUE(Wrapper6::isInitialized());

    // Destroy
    Wrapper6::destroy();
    EXPECT_FALSE(Wrapper6::isInitialized());
}

// Test isInitialized state tracking
TEST(StaticObjectWrapperTest, IsInitializedStateTracking) {
    // Initial state
    Wrapper7::destroy();
    EXPECT_FALSE(Wrapper7::isInitialized());

    // After init
    const auto config_pair = std::make_pair(TestConfig{"state_test", 789}, TestConfigWrapper7{});
    Wrapper7::init(config_pair);
    EXPECT_TRUE(Wrapper7::isInitialized());

    // After destroy
    Wrapper7::destroy();
    EXPECT_FALSE(Wrapper7::isInitialized());
}

// Test multiple calls to init (only first should take effect)
TEST(StaticObjectWrapperTest, MultipleInitCalls) {
    // Clean state
    Wrapper8::destroy();

    // First init
    const auto first_config = std::make_pair(TestConfig{"first", 1}, TestConfigWrapper8{});
    Wrapper8::init(first_config);

    // Second init should be ignored (std::call_once guarantee)
    const auto second_config = std::make_pair(TestConfig{"second", 2}, TestConfigWrapper8{});
    Wrapper8::init(second_config);

    // Should still have first values
    const auto &config = Wrapper8::get();
    EXPECT_EQ(config.first.name, "first");
    EXPECT_EQ(config.first.value, 1);

    // Cleanup
    Wrapper8::destroy();
}

/**
 * @brief Test that constructor is deleted (compile-time check)
 * @details Verifies StaticObjectWrapper cannot be instantiated
 */
TEST(StaticObjectWrapperTest, ConstructorDeleted) {
    // This test verifies at compile time that StaticObjectWrapper cannot be instantiated
    static_assert(std::is_constructible_v<StaticObjectWrapper<TestConfig> > == false,
                  "StaticObjectWrapper should not be constructible");
}

// Test copy constructor is deleted (compile-time check)
TEST(StaticObjectWrapperTest, CopyConstructorDeleted) {
    static_assert(std::is_copy_constructible_v<StaticObjectWrapper<TestConfig> > == false,
                  "StaticObjectWrapper should not be copy constructible");
}

// Test copy assignment is deleted (compile-time check)
TEST(StaticObjectWrapperTest, CopyAssignmentDeleted) {
    static_assert(std::is_copy_assignable_v<StaticObjectWrapper<TestConfig> > == false,
                  "StaticObjectWrapper should not be copy assignable");
}