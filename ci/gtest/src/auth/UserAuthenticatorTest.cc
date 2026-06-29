/**
 * @file UserAuthenticatorTest.cc
 * @brief Unit tests for the UserAuthenticator class
 * @details Tests cover user registration, authentication, password management,
 *          account lockout, and boundary conditions.
 */

#include <gtest/gtest.h>

#include <cppforge/auth/UserAuthenticator.hpp>
#include <cppforge/auth/AuthenticationException.hpp>

using namespace cppforge::auth;

/**
 * @brief Test fixture for UserAuthenticatorTest tests
 * @details Uses SQLite :memory: database for isolation; uses a relaxed
 *          password policy so basic tests don't require complex passwords.
 */
class UserAuthenticatorTest : public testing::Test
{
protected:
    static std::unique_ptr<UserAuthenticator> authenticator_;

    static void SetUpTestSuite()
    {
        // Relaxed policy for basic auth tests
        const PasswordPolicy relaxed_policy(
            1,    // min_length
            100,  // max_length
            false,// require_uppercase
            false,// require_lowercase
            false,// require_digits
            false,// require_special
            5     // max_login_attempts
        );
        authenticator_ = std::make_unique<UserAuthenticator>(":memory:", relaxed_policy);
    }

    static void TearDownTestSuite()
    {
        authenticator_.reset();
    }

    void SetUp() override
    {
        // Clean up any users created in previous tests
        cleanupAllUsers();
    }

    void TearDown() override
    {
        cleanupAllUsers();
    }

private:
    static void cleanupAllUsers()
    {
        if (!authenticator_)
        {
            return;
        }
        auto& users = authenticator_->get_users();
        auto& mutex = authenticator_->get_users_mutex();
        std::lock_guard lock(mutex);
        users.clear();
    }
};

std::unique_ptr<UserAuthenticator> UserAuthenticatorTest::authenticator_ = nullptr;

// ============================================================================
// Constructor Tests
// ============================================================================

/**
 * @brief Test constructor with default policy
 * @details Verifies that authenticator can be created with default policy
 */
TEST_F(UserAuthenticatorTest, Constructor_DefaultPolicy)
{
    EXPECT_NO_THROW((UserAuthenticator(":memory:")));
}

/**
 * @brief Test constructor with custom policy
 * @details Verifies that authenticator can be created with explicit policy
 */
TEST_F(UserAuthenticatorTest, Constructor_CustomPolicy)
{
    const PasswordPolicy custom_policy(4, 20, false, false, false, false, 3);
    EXPECT_NO_THROW((UserAuthenticator(":memory:", custom_policy)));
}

// ============================================================================
// register_user Tests
// ============================================================================

/**
 * @brief Test register_user success
 * @details Verifies that a valid user can be registered
 */
TEST_F(UserAuthenticatorTest, RegisterUser_Success)
{
    EXPECT_TRUE(authenticator_->register_user("test_user", "password123"));
    EXPECT_TRUE(authenticator_->user_exists("test_user"));
}

/**
 * @brief Test register_user with duplicate username
 * @details Verifies that registering an existing username throws
 */
TEST_F(UserAuthenticatorTest, RegisterUser_Duplicate)
{
    EXPECT_TRUE(authenticator_->register_user("existing", "password123"));
    EXPECT_THROW(
        static_cast<void>(authenticator_->register_user("existing", "other_pass")),
        AuthenticationException
    );
}

/**
 * @brief Test register_user with invalid username (too short)
 * @details Verifies that a username shorter than 3 characters is rejected
 */
TEST_F(UserAuthenticatorTest, RegisterUser_UsernameTooShort)
{
    EXPECT_THROW(
        static_cast<void>(authenticator_->register_user("ab", "password123")),
        AuthenticationException
    );
}

/**
 * @brief Test register_user with invalid username (too long)
 * @details Verifies that a username longer than 20 characters is rejected
 */
TEST_F(UserAuthenticatorTest, RegisterUser_UsernameTooLong)
{
    const std::string long_username(21, 'a');
    EXPECT_THROW(
        static_cast<void>(authenticator_->register_user(long_username, "password123")),
        AuthenticationException
    );
}

/**
 * @brief Test register_user with invalid username (special characters)
 * @details Verifies that username with special characters is rejected
 */
TEST_F(UserAuthenticatorTest, RegisterUser_UsernameSpecialChars)
{
    EXPECT_THROW(
        static_cast<void>(authenticator_->register_user("user@name", "password123")),
        AuthenticationException
    );
    EXPECT_THROW(
        static_cast<void>(authenticator_->register_user("user name", "password123")),
        AuthenticationException
    );
    EXPECT_THROW(
        static_cast<void>(authenticator_->register_user("user.name", "password123")),
        AuthenticationException
    );
}

/**
 * @brief Test register_user with valid username containing underscore and hyphen
 * @details Verifies that underscore and hyphen are allowed in usernames
 */
TEST_F(UserAuthenticatorTest, RegisterUser_UsernameUnderscoreHyphen)
{
    EXPECT_TRUE(authenticator_->register_user("user_name", "password123"));
    EXPECT_TRUE(authenticator_->register_user("user-name", "password123"));
}

/**
 * @brief Test register_user with empty username
 * @details Verifies that empty username is rejected
 */
TEST_F(UserAuthenticatorTest, RegisterUser_EmptyUsername)
{
    EXPECT_THROW(
        static_cast<void>(authenticator_->register_user("", "password123")),
        AuthenticationException
    );
}

// ============================================================================
// authenticate Tests
// ============================================================================

/**
 * @brief Test authenticate success
 * @details Verifies that valid credentials return success
 */
TEST_F(UserAuthenticatorTest, Authenticate_Success)
{
    EXPECT_TRUE(authenticator_->register_user("alice", "password123"));

    const auto result = authenticator_->authenticate("alice", "password123");
    EXPECT_TRUE(result.is_success());
    EXPECT_TRUE(result.error_message.empty());
    EXPECT_NE(result.user, nullptr);
}

/**
 * @brief Test authenticate with wrong password
 * @details Verifies that wrong password returns failure
 */
TEST_F(UserAuthenticatorTest, Authenticate_WrongPassword)
{
    EXPECT_TRUE(authenticator_->register_user("bob", "password123"));

    const auto result = authenticator_->authenticate("bob", "wrong_pass");
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.error_message, "Invalid password");
    EXPECT_EQ(result.user, nullptr);
}

/**
 * @brief Test authenticate with non-existent user
 * @details Verifies that non-existent user returns failure
 */
TEST_F(UserAuthenticatorTest, Authenticate_NonExistentUser)
{
    const auto result = authenticator_->authenticate("nobody", "password123");
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.error_message, "User not found");
    EXPECT_EQ(result.user, nullptr);
}

/**
 * @brief Test authenticate with empty username
 * @details Verifies that empty username returns failure
 */
TEST_F(UserAuthenticatorTest, Authenticate_EmptyUsername)
{
    const auto result = authenticator_->authenticate("", "password123");
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.user, nullptr);
}

/**
 * @brief Test authenticate with empty password
 * @details Verifies that empty password returns failure
 */
TEST_F(UserAuthenticatorTest, Authenticate_EmptyPassword)
{
    EXPECT_TRUE(authenticator_->register_user("charlie", "password123"));

    const auto result = authenticator_->authenticate("charlie", "");
    EXPECT_FALSE(result.is_success());
    EXPECT_EQ(result.error_message, "Invalid password");
}

// ============================================================================
// Account Lockout Tests
// ============================================================================

/**
 * @brief Test account locks after max failed attempts
 * @details Verifies that after max_login_attempts failed authentications,
 *          the account becomes locked
 */
TEST_F(UserAuthenticatorTest, Authenticate_AccountLockout)
{
    EXPECT_TRUE(authenticator_->register_user("dave", "password123"));

    // Attempt with wrong password up to max attempts
    for (int i = 0; i < 5; ++i)
    {
        const auto result = authenticator_->authenticate("dave", "wrong_pass");
        EXPECT_FALSE(result.is_success());
        EXPECT_EQ(result.error_message, "Invalid password");
    }

    // Next attempt should report lockout
    const auto locked_result = authenticator_->authenticate("dave", "password123");
    EXPECT_FALSE(locked_result.is_success());
    EXPECT_TRUE(locked_result.error_message.find("locked") != std::string::npos);
}

/**
 * @brief Test successful login resets failed attempts
 * @details Verifies that a successful login between failures resets the counter
 */
TEST_F(UserAuthenticatorTest, Authenticate_SuccessResetsLockout)
{
    EXPECT_TRUE(authenticator_->register_user("eve", "password123"));

    // Few failed attempts
    for (int i = 0; i < 3; ++i)
    {
        static_cast<void>(authenticator_->authenticate("eve", "wrong_pass"));
    }

    // Successful login
    auto result = authenticator_->authenticate("eve", "password123");
    EXPECT_TRUE(result.is_success());

    // Failed counter should be reset; remaining attempts should be available
    for (int i = 0; i < 5; ++i)
    {
        result = authenticator_->authenticate("eve", "wrong_pass");
        EXPECT_FALSE(result.is_success());
    }

    // Now should be locked again
    result = authenticator_->authenticate("eve", "password123");
    EXPECT_FALSE(result.is_success());
    EXPECT_TRUE(result.error_message.find("locked") != std::string::npos);
}

// ============================================================================
// change_password Tests
// ============================================================================

/**
 * @brief Test change_password success
 * @details Verifies that password can be changed with correct current password
 */
TEST_F(UserAuthenticatorTest, ChangePassword_Success)
{
    EXPECT_TRUE(authenticator_->register_user("frank", "current_pass"));

    EXPECT_TRUE(authenticator_->change_password("frank", "current_pass", "new_pass_123"));

    // Old password should no longer work
    const auto old_result = authenticator_->authenticate("frank", "current_pass");
    EXPECT_FALSE(old_result.is_success());

    // New password should work
    const auto new_result = authenticator_->authenticate("frank", "new_pass_123");
    EXPECT_TRUE(new_result.is_success());
}

/**
 * @brief Test change_password with wrong current password
 * @details Verifies that wrong current password throws
 */
TEST_F(UserAuthenticatorTest, ChangePassword_WrongCurrentPassword)
{
    EXPECT_TRUE(authenticator_->register_user("grace", "correct_pass"));

    EXPECT_THROW(
        static_cast<void>(authenticator_->change_password("grace", "wrong_pass", "new_pass_123")),
        AuthenticationException
    );

    // Original password must remain valid
    const auto result = authenticator_->authenticate("grace", "correct_pass");
    EXPECT_TRUE(result.is_success());
}

/**
 * @brief Test change_password for non-existent user
 * @details Verifies that changing password for non-existent user throws
 */
TEST_F(UserAuthenticatorTest, ChangePassword_NonExistentUser)
{
    EXPECT_THROW(
        static_cast<void>(authenticator_->change_password("ghost", "old", "new_pass_123")),
        AuthenticationException
    );
}

// ============================================================================
// reset_password Tests
// ============================================================================

/**
 * @brief Test reset_password success
 * @details Verifies that admin password reset works
 */
TEST_F(UserAuthenticatorTest, ResetPassword_Success)
{
    EXPECT_TRUE(authenticator_->register_user("heidi", "original_pass"));

    EXPECT_TRUE(authenticator_->reset_password("heidi", "reset_pass_123"));

    // Old password should no longer work
    const auto old_result = authenticator_->authenticate("heidi", "original_pass");
    EXPECT_FALSE(old_result.is_success());

    // New password should work
    const auto new_result = authenticator_->authenticate("heidi", "reset_pass_123");
    EXPECT_TRUE(new_result.is_success());
}

/**
 * @brief Test reset_password for non-existent user
 * @details Verifies that resetting password for non-existent user throws
 */
TEST_F(UserAuthenticatorTest, ResetPassword_NonExistentUser)
{
    EXPECT_THROW(
        static_cast<void>(authenticator_->reset_password("ghost", "new_pass_123")),
        AuthenticationException
    );
}

// ============================================================================
// user_exists Tests
// ============================================================================

/**
 * @brief Test user_exists returns true for existing user
 * @details Verifies user exists after registration
 */
TEST_F(UserAuthenticatorTest, UserExists_True)
{
    EXPECT_TRUE(authenticator_->register_user("ivan", "password123"));
    EXPECT_TRUE(authenticator_->user_exists("ivan"));
}

/**
 * @brief Test user_exists returns false for non-existent user
 * @details Verifies user does not exist before registration
 */
TEST_F(UserAuthenticatorTest, UserExists_False)
{
    EXPECT_FALSE(authenticator_->user_exists("nobody_here"));
}

/**
 * @brief Test user_exists with empty string
 * @details Verifies that empty string returns false
 */
TEST_F(UserAuthenticatorTest, UserExists_Empty)
{
    EXPECT_FALSE(authenticator_->user_exists(""));
}

// ============================================================================
// delete_user Tests
// ============================================================================

/**
 * @brief Test delete_user success
 * @details Verifies that a registered user can be deleted
 */
TEST_F(UserAuthenticatorTest, DeleteUser_Success)
{
    EXPECT_TRUE(authenticator_->register_user("judy", "password123"));
    EXPECT_TRUE(authenticator_->user_exists("judy"));

    EXPECT_TRUE(authenticator_->delete_user("judy"));
    EXPECT_FALSE(authenticator_->user_exists("judy"));
}

/**
 * @brief Test delete_user for non-existent user
 * @details Verifies that deleting non-existent user returns false
 */
TEST_F(UserAuthenticatorTest, DeleteUser_NonExistent)
{
    EXPECT_FALSE(authenticator_->delete_user("nobody"));
}

// ============================================================================
// Password Policy Tests
// ============================================================================

/**
 * @brief Test password validation with strict policy
 * @details Verifies that weak passwords are rejected when strict policy is set
 */
TEST_F(UserAuthenticatorTest, PasswordPolicy_RejectsWeakPassword)
{
    // Create authenticator with strict default policy
    const PasswordPolicy strict_policy(8, 64, true, true, true, true, 5);
    UserAuthenticator strict_auth(":memory:", strict_policy);

    // Weak password without uppercase, digit, or special
    EXPECT_THROW(
        static_cast<void>(strict_auth.register_user("weak_user", "weakpass")),
        AuthenticationException
    );
}

/**
 * @brief Test password policy can be updated after construction
 * @details Verifies that set_password_policy changes validation rules
 */
TEST_F(UserAuthenticatorTest, SetPasswordPolicy_UpdatesValidation)
{
    // Create with strict policy
    const PasswordPolicy strict_policy(8, 64, true, true, true, true, 5);
    UserAuthenticator auth(":memory:", strict_policy);

    // Weak password fails with strict policy
    EXPECT_THROW(
        static_cast<void>(auth.register_user("user1", "weak")),
        AuthenticationException
    );

    // Switch to relaxed policy
    const PasswordPolicy relaxed_policy(1, 100, false, false, false, false, 5);
    auth.set_password_policy(relaxed_policy);

    // Same weak password now works
    EXPECT_TRUE(auth.register_user("user1", "weak"));
}

// ============================================================================
// Boundary Tests
// ============================================================================

/**
 * @brief Test username at exact boundary (3 chars)
 * @details Verifies that a 3-character username is accepted
 */
TEST_F(UserAuthenticatorTest, UsernameBoundary_MinLength)
{
    EXPECT_TRUE(authenticator_->register_user("abc", "password123"));
    EXPECT_TRUE(authenticator_->user_exists("abc"));
}

/**
 * @brief Test username at exact boundary (20 chars)
 * @details Verifies that a 20-character username is accepted
 */
TEST_F(UserAuthenticatorTest, UsernameBoundary_MaxLength)
{
    const std::string username(20, 'a');
    EXPECT_TRUE(authenticator_->register_user(username, "password123"));
    EXPECT_TRUE(authenticator_->user_exists(username));
}

/**
 * @brief Test long password
 * @details Verifies that long passwords within policy bounds are handled correctly
 */
TEST_F(UserAuthenticatorTest, LongPassword)
{
    const std::string long_password(95, 'x');
    EXPECT_TRUE(authenticator_->register_user("longpass_user", long_password));

    const auto result = authenticator_->authenticate("longpass_user", long_password);
    EXPECT_TRUE(result.is_success());
}

/**
 * @brief Test multiple users can be registered and managed independently
 * @details Verifies that the authenticator handles multiple users correctly
 */
TEST_F(UserAuthenticatorTest, MultipleUsers)
{
    EXPECT_TRUE(authenticator_->register_user("user_a", "pass_a_123"));
    EXPECT_TRUE(authenticator_->register_user("user_b", "pass_b_123"));
    EXPECT_TRUE(authenticator_->register_user("user_c", "pass_c_123"));

    EXPECT_TRUE(authenticator_->user_exists("user_a"));
    EXPECT_TRUE(authenticator_->user_exists("user_b"));
    EXPECT_TRUE(authenticator_->user_exists("user_c"));

    // Delete middle user
    EXPECT_TRUE(authenticator_->delete_user("user_b"));
    EXPECT_FALSE(authenticator_->user_exists("user_b"));
    EXPECT_TRUE(authenticator_->user_exists("user_a"));
    EXPECT_TRUE(authenticator_->user_exists("user_c"));

    // Re-register deleted user
    EXPECT_TRUE(authenticator_->register_user("user_b", "new_pass_456"));
    EXPECT_TRUE(authenticator_->user_exists("user_b"));
}

/**
 * @brief Test authenticate after password reset
 * @details Verifies that password reset allows new password to authenticate
 */
TEST_F(UserAuthenticatorTest, AuthenticateAfterReset)
{
    EXPECT_TRUE(authenticator_->register_user("reset_user", "initial_pass"));

    EXPECT_TRUE(authenticator_->reset_password("reset_user", "new_pass_456"));

    auto result = authenticator_->authenticate("reset_user", "new_pass_456");
    EXPECT_TRUE(result.is_success());

    // Old password no longer works
    result = authenticator_->authenticate("reset_user", "initial_pass");
    EXPECT_FALSE(result.is_success());
}

/**
 * @brief Test successful authentication returns valid user pointer
 * @details Verifies that the user pointer in AuthResult is valid
 */
TEST_F(UserAuthenticatorTest, Authenticate_ReturnedUserPointer)
{
    EXPECT_TRUE(authenticator_->register_user("pointer_user", "password123"));

    const auto result = authenticator_->authenticate("pointer_user", "password123");
    ASSERT_TRUE(result.is_success());
    ASSERT_NE(result.user, nullptr);
    EXPECT_EQ(result.user->get_username(), "pointer_user");
}

/**
 * @brief Test authenticate after delete and re-register
 * @details Verifies that deleting and re-registering works correctly
 */
TEST_F(UserAuthenticatorTest, DeleteAndReRegister)
{
    EXPECT_TRUE(authenticator_->register_user("cyclical", "pass_one"));
    EXPECT_TRUE(authenticator_->delete_user("cyclical"));

    // Re-register with different password
    EXPECT_TRUE(authenticator_->register_user("cyclical", "pass_two"));

    // New password should work
    auto result = authenticator_->authenticate("cyclical", "pass_two");
    EXPECT_TRUE(result.is_success());

    // Old password should not work
    result = authenticator_->authenticate("cyclical", "pass_one");
    EXPECT_FALSE(result.is_success());
}
