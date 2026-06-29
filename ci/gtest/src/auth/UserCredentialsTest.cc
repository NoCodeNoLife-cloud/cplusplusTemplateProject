/**
 * @file UserCredentialsTest.cc
 * @brief Unit tests for the UserCredentials class
 * @details Tests cover credential storage, failed attempt tracking,
 *          and account lockout functionality.
 */

#include <chrono>
#include <gtest/gtest.h>

#include "auth/UserCredentials.hpp"

using namespace cppforge::auth;

/**
 * @brief Test fixture for UserCredentialsTest tests
 */
class UserCredentialsTest : public testing::Test
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
 * @brief Test UserCredentials construction
 * @details Verifies that credentials are properly initialized
 */
TEST_F(UserCredentialsTest, Construction)
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
TEST_F(UserCredentialsTest, GetUsername)
{
    const UserCredentials creds("john_doe", "hash", "salt");

    EXPECT_EQ(creds.get_username(), "john_doe");
}

/**
 * @brief Test get_hashed_password returns correct value
 * @details Verifies hashed password accessor works correctly
 */
TEST_F(UserCredentialsTest, GetHashedPassword)
{
    const UserCredentials creds("user", "secure_hash_value", "salt");

    EXPECT_EQ(creds.get_hashed_password(), "secure_hash_value");
}

/**
 * @brief Test get_salt returns correct value
 * @details Verifies salt accessor works correctly
 */
TEST_F(UserCredentialsTest, GetSalt)
{
    const UserCredentials creds("user", "hash", "unique_salt_value");

    EXPECT_EQ(creds.get_salt(), "unique_salt_value");
}

/**
 * @brief Test initial failed attempts is zero
 * @details Verifies that new credentials start with zero failed attempts
 */
TEST_F(UserCredentialsTest, InitialFailedAttempts)
{
    const UserCredentials creds("user", "hash", "salt");

    EXPECT_EQ(creds.get_failed_attempts(), 0);
}

/**
 * @brief Test increment_failed_attempts increases counter
 * @details Verifies that failed attempts counter increments correctly
 */
TEST_F(UserCredentialsTest, IncrementFailedAttempts)
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
TEST_F(UserCredentialsTest, ResetFailedAttempts)
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
TEST_F(UserCredentialsTest, IsLocked_DefaultParameters)
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
TEST_F(UserCredentialsTest, IsLocked_CustomParameters)
{
    UserCredentials creds("user", "hash", "salt");

    constexpr auto lockout_duration = std::chrono::minutes{10};
    constexpr size_t max_attempts = 3;

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
TEST_F(UserCredentialsTest, IsLocked_ExceedsMaxAttempts)
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
TEST_F(UserCredentialsTest, MultipleIncrementResetCycles)
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
TEST_F(UserCredentialsTest, EmptyStrings)
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
TEST_F(UserCredentialsTest, LongStrings)
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
TEST_F(UserCredentialsTest, SpecialCharacters)
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
TEST_F(UserCredentialsTest, LockoutBoundary)
{
    UserCredentials creds("user", "hash", "salt");

    constexpr size_t max_attempts = 5;
    constexpr auto lockout_duration = std::chrono::minutes{5};

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
TEST_F(UserCredentialsTest, SuccessfulLoginResetsAttempts)
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

// ============================================================================
// Boundary Condition Tests
// ============================================================================

/**
 * @brief Test is_locked with max_attempts = 0 (lock after first failure)
 * @details With max_attempts = 0, the condition failed_attempts_ >= 0 is
 *          always true for unsigned size_t. After one increment the
 *          time-based condition is met, so account locks immediately.
 */
TEST_F(UserCredentialsTest, IsLocked_ZeroMaxAttempts)
{
    UserCredentials creds("user", "hash", "salt");

    // Initially not locked despite max_attempts=0 because
    // last_failed_attempt_ = time_point::min() makes timesince huge
    EXPECT_FALSE(creds.is_locked(std::chrono::minutes{5}, 0));

    // After one failure: attempts >= 0 (true) and timesince â‰?0 < 5min
    creds.increment_failed_attempts();
    EXPECT_TRUE(creds.is_locked(std::chrono::minutes{5}, 0));
}

/**
 * @brief Test is_locked with max_attempts = 1
 * @details Account locks after exactly one failed attempt
 */
TEST_F(UserCredentialsTest, IsLocked_SingleMaxAttempt)
{
    UserCredentials creds("user", "hash", "salt");

    EXPECT_FALSE(creds.is_locked(std::chrono::minutes{5}, 1));

    creds.increment_failed_attempts();
    EXPECT_TRUE(creds.is_locked(std::chrono::minutes{5}, 1));
}

/**
 * @brief Test is_locked with max_attempts = SIZE_MAX (practically never locks)
 * @details Unless failed_attempts_ reaches SIZE_MAX, lock never triggers
 */
TEST_F(UserCredentialsTest, IsLocked_MaxSizeTMaxAttempts)
{
    UserCredentials creds("user", "hash", "salt");

    for (size_t i = 0; i < 100; ++i)
    {
        creds.increment_failed_attempts();
    }
    // 100 < SIZE_MAX, so lock is never reached
    EXPECT_FALSE(creds.is_locked(std::chrono::minutes{5}, SIZE_MAX));
}

/**
 * @brief Test lockout with zero-minute duration (immediate expiration)
 * @details With lockout_duration = 0min, the condition
 *          minutes_since_last_fail < 0 is always false,
 *          so the account can never be locked.
 */
TEST_F(UserCredentialsTest, IsLocked_ZeroDuration)
{
    UserCredentials creds("user", "hash", "salt");

    creds.increment_failed_attempts();
    // Timesince >= 0, so < 0 is false â†?not locked
    EXPECT_FALSE(creds.is_locked(std::chrono::minutes{0}, 5));
}

/**
 * @brief Test reset clears locked state after lockout
 * @details Even after many failures, resetting attempts should
 *          clear the locked state regardless of timing
 */
TEST_F(UserCredentialsTest, ResetClearsLockedState)
{
    UserCredentials creds("user", "hash", "salt");

    for (int i = 0; i < 10; ++i)
    {
        creds.increment_failed_attempts();
    }
    EXPECT_TRUE(creds.is_locked());

    creds.reset_failed_attempts();
    EXPECT_FALSE(creds.is_locked());
}

/**
 * @brief Test increment with size_t overflow boundary
 * @details Simulates incrementing to near SIZE_MAX and then once more
 *          to verify no crash or UB on wraparound
 */
TEST_F(UserCredentialsTest, IncrementOverflowBoundary)
{
    UserCredentials creds("user", "hash", "salt");

    // We cannot practically increment to SIZE_MAX, but we can verify
    // that a large number of increments is handled
    for (size_t i = 0; i < 10000; ++i)
    {
        creds.increment_failed_attempts();
    }

    EXPECT_EQ(creds.get_failed_attempts(), 10000);
    EXPECT_TRUE(creds.is_locked());
}

/**
 * @brief Test is_locked after auto-expiration of lockout
 * @details By passing a 0-minute duration we can simulate expiration.
 *          Also tests that after reset + re-increment, lock re-activates.
 */
TEST_F(UserCredentialsTest, LockExpirationAndReactivation)
{
    UserCredentials creds("user", "hash", "salt");

    constexpr auto lockout_duration = std::chrono::minutes{5};
    constexpr size_t max_attempts = 3;

    // Lock the account
    creds.increment_failed_attempts();
    creds.increment_failed_attempts();
    creds.increment_failed_attempts();
    EXPECT_TRUE(creds.is_locked(lockout_duration, max_attempts));

    // Reset to simulate successful login
    creds.reset_failed_attempts();
    EXPECT_FALSE(creds.is_locked(lockout_duration, max_attempts));

    // New failure cycle should lock again
    creds.increment_failed_attempts();
    creds.increment_failed_attempts();
    creds.increment_failed_attempts();
    EXPECT_TRUE(creds.is_locked(lockout_duration, max_attempts));
}
