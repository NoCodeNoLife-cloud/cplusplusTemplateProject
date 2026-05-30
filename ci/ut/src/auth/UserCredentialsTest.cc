/**
 * @file UserCredentialsTest.cc
 * @brief Unit tests for the UserCredentials class
 * @details Tests cover credential storage, failed attempt tracking,
 *          and account lockout functionality.
 */

#include <gtest/gtest.h>
#include "auth/UserCredentials.hpp"
#include <chrono>

using namespace common::auth;

/**
 * @brief Test UserCredentials construction
 * @details Verifies that credentials are properly initialized
 */
TEST(UserCredentialsTest, Construction)
{
    const UserCredentials creds("testuser", "hashed_pass_123", "salt_456");

    EXPECT_EQ(creds.get_username(), "testuser");
    EXPECT_EQ(creds.get_hashed_password(), "hashed_pass_123");
    EXPECT_EQ(creds.get_salt(), "salt_456");
    EXPECT_EQ(creds.get_failed_attempts(), 0);
}

/**
 * @brief Test get_username returns correct value
 * @details Verifies username accessor works correctly
 */
TEST(UserCredentialsTest, GetUsername)
{
    const UserCredentials creds("john_doe", "hash", "salt");

    EXPECT_EQ(creds.get_username(), "john_doe");
}

/**
 * @brief Test get_hashed_password returns correct value
 * @details Verifies hashed password accessor works correctly
 */
TEST(UserCredentialsTest, GetHashedPassword)
{
    const UserCredentials creds("user", "secure_hash_value", "salt");

    EXPECT_EQ(creds.get_hashed_password(), "secure_hash_value");
}

/**
 * @brief Test get_salt returns correct value
 * @details Verifies salt accessor works correctly
 */
TEST(UserCredentialsTest, GetSalt)
{
    const UserCredentials creds("user", "hash", "unique_salt_value");

    EXPECT_EQ(creds.get_salt(), "unique_salt_value");
}

/**
 * @brief Test initial failed attempts is zero
 * @details Verifies that new credentials start with zero failed attempts
 */
TEST(UserCredentialsTest, InitialFailedAttempts)
{
    const UserCredentials creds("user", "hash", "salt");

    EXPECT_EQ(creds.get_failed_attempts(), 0);
}

/**
 * @brief Test increment_failed_attempts increases counter
 * @details Verifies that failed attempts counter increments correctly
 */
TEST(UserCredentialsTest, IncrementFailedAttempts)
{
    UserCredentials creds("user", "hash", "salt");

    EXPECT_EQ(creds.get_failed_attempts(), 0);

    creds.increment_failed_attempts();
    EXPECT_EQ(creds.get_failed_attempts(), 1);

    creds.increment_failed_attempts();
    EXPECT_EQ(creds.get_failed_attempts(), 2);

    creds.increment_failed_attempts();
    EXPECT_EQ(creds.get_failed_attempts(), 3);
}

/**
 * @brief Test reset_failed_attempts clears counter
 * @details Verifies that failed attempts can be reset to zero
 */
TEST(UserCredentialsTest, ResetFailedAttempts)
{
    UserCredentials creds("user", "hash", "salt");

    // Increment several times
    for (int i = 0; i < 5; ++i)
    {
        creds.increment_failed_attempts();
    }
    EXPECT_EQ(creds.get_failed_attempts(), 5);

    // Reset
    creds.reset_failed_attempts();
    EXPECT_EQ(creds.get_failed_attempts(), 0);
}

/**
 * @brief Test is_locked with default parameters
 * @details Verifies account lockout with default max attempts (5)
 */
TEST(UserCredentialsTest, IsLocked_DefaultParameters)
{
    UserCredentials creds("user", "hash", "salt");

    // Not locked initially
    EXPECT_FALSE(creds.is_locked());

    // Add some failed attempts but not enough to lock
    for (int i = 0; i < 4; ++i)
    {
        creds.increment_failed_attempts();
    }
    EXPECT_FALSE(creds.is_locked());

    // Reach max attempts - should be locked
    creds.increment_failed_attempts();
    EXPECT_TRUE(creds.is_locked());
}

/**
 * @brief Test is_locked with custom duration and max attempts
 * @details Verifies account lockout with custom parameters
 */
TEST(UserCredentialsTest, IsLocked_CustomParameters)
{
    UserCredentials creds("user", "hash", "salt");

    const auto lockout_duration = std::chrono::minutes{10};
    const size_t max_attempts = 3;

    // Not locked initially
    EXPECT_FALSE(creds.is_locked(lockout_duration, max_attempts));

    // Add 2 failed attempts (below threshold)
    creds.increment_failed_attempts();
    creds.increment_failed_attempts();
    EXPECT_FALSE(creds.is_locked(lockout_duration, max_attempts));

    // Add 3rd failed attempt (reaches threshold)
    creds.increment_failed_attempts();
    EXPECT_TRUE(creds.is_locked(lockout_duration, max_attempts));
}

/**
 * @brief Test is_locked after exceeding max attempts
 * @details Verifies that account remains locked after exceeding limit
 */
TEST(UserCredentialsTest, IsLocked_ExceedsMaxAttempts)
{
    UserCredentials creds("user", "hash", "salt");

    // Exceed max attempts
    for (int i = 0; i < 10; ++i)
    {
        creds.increment_failed_attempts();
    }

    EXPECT_TRUE(creds.is_locked());
    EXPECT_EQ(creds.get_failed_attempts(), 10);
}

/**
 * @brief Test multiple increment and reset cycles
 * @details Verifies that increment/reset cycle works correctly multiple times
 */
TEST(UserCredentialsTest, MultipleIncrementResetCycles)
{
    UserCredentials creds("user", "hash", "salt");

    // First cycle
    for (int i = 0; i < 3; ++i)
    {
        creds.increment_failed_attempts();
    }
    EXPECT_EQ(creds.get_failed_attempts(), 3);

    creds.reset_failed_attempts();
    EXPECT_EQ(creds.get_failed_attempts(), 0);

    // Second cycle
    for (int i = 0; i < 5; ++i)
    {
        creds.increment_failed_attempts();
    }
    EXPECT_EQ(creds.get_failed_attempts(), 5);

    creds.reset_failed_attempts();
    EXPECT_EQ(creds.get_failed_attempts(), 0);
}

/**
 * @brief Test credentials with empty strings
 * @details Verifies that credentials can be created with empty strings
 */
TEST(UserCredentialsTest, EmptyStrings)
{
    const UserCredentials creds("", "", "");

    EXPECT_EQ(creds.get_username(), "");
    EXPECT_EQ(creds.get_hashed_password(), "");
    EXPECT_EQ(creds.get_salt(), "");
    EXPECT_EQ(creds.get_failed_attempts(), 0);
}

/**
 * @brief Test credentials with long strings
 * @details Verifies that credentials handle long strings correctly
 */
TEST(UserCredentialsTest, LongStrings)
{
    const std::string long_username(1000, 'u');
    const std::string long_hash(1000, 'h');
    const std::string long_salt(1000, 's');

    const UserCredentials creds(long_username, long_hash, long_salt);

    EXPECT_EQ(creds.get_username(), long_username);
    EXPECT_EQ(creds.get_hashed_password(), long_hash);
    EXPECT_EQ(creds.get_salt(), long_salt);
}

/**
 * @brief Test credentials with special characters
 * @details Verifies that credentials handle special characters correctly
 */
TEST(UserCredentialsTest, SpecialCharacters)
{
    const UserCredentials creds(
        "user@domain.com",
        "hash!@#$%^&*()",
        "salt_with_special!@#"
    );

    EXPECT_EQ(creds.get_username(), "user@domain.com");
    EXPECT_EQ(creds.get_hashed_password(), "hash!@#$%^&*()");
    EXPECT_EQ(creds.get_salt(), "salt_with_special!@#");
}

/**
 * @brief Test lockout behavior at exact boundary
 * @details Verifies lockout occurs exactly at max_attempts threshold
 */
TEST(UserCredentialsTest, LockoutBoundary)
{
    UserCredentials creds("user", "hash", "salt");

    const size_t max_attempts = 5;
    const auto lockout_duration = std::chrono::minutes{5};

    // Try up to boundary
    for (size_t i = 0; i < max_attempts - 1; ++i)
    {
        creds.increment_failed_attempts();
        EXPECT_FALSE(creds.is_locked(lockout_duration, max_attempts));
    }

    // At boundary - should lock
    creds.increment_failed_attempts();
    EXPECT_TRUE(creds.is_locked(lockout_duration, max_attempts));
}

/**
 * @brief Test that successful login resets failed attempts
 * @details Simulates successful authentication by resetting attempts
 */
TEST(UserCredentialsTest, SuccessfulLoginResetsAttempts)
{
    UserCredentials creds("user", "hash", "salt");

    // Simulate failed attempts
    for (int i = 0; i < 3; ++i)
    {
        creds.increment_failed_attempts();
    }
    EXPECT_EQ(creds.get_failed_attempts(), 3);

    // Simulate successful login - reset attempts
    creds.reset_failed_attempts();
    EXPECT_EQ(creds.get_failed_attempts(), 0);
    EXPECT_FALSE(creds.is_locked());
}
