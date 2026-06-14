/**
 * @file PasswordSQLTest.cc
 * @brief Unit tests for the PasswordSQL class
 * @details Tests cover user registration, authentication, password management,
 *          user queries, and edge cases with automatic database cleanup.
 */

#include <gtest/gtest.h>

#include "sql/PasswordSQL.hpp"

using namespace common::sql;

/**
 * @brief Test fixture for PasswordSQL tests
 * @details Uses SQLite :memory: database for isolation; no file cleanup needed.
 */
class PasswordSQLTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        instance_ = std::make_unique<PasswordSQL>(":memory:");
    }

    static void TearDownTestSuite()
    {
        instance_.reset();
    }

    void SetUp() override
    {
        // Ensure clean state before each test
        cleanupAllUsers();
    }

    void TearDown() override
    {
        // Ensure clean state after each test
        cleanupAllUsers();
    }

    static std::unique_ptr<PasswordSQL> instance_;

private:
    static void cleanupAllUsers()
    {
        if (!instance_)
        {
            return;
        }
        // Delete users that may have been created by tests
        for (const auto& user : instance_->GetAllUsers())
        {
            instance_->DeleteUser(user);
        }
    }
};

std::unique_ptr<PasswordSQL> PasswordSQLTest::instance_ = nullptr;

/**
 * @brief Test successful user registration
 * @details Registers a new user with valid credentials and verifies the user
 *          exists in the database after registration
 */
TEST_F(PasswordSQLTest, RegisterUser_Success)
{
    EXPECT_TRUE(instance_->RegisterUser("alice", "pass123"));
    EXPECT_TRUE(instance_->UserExists("alice"));
}

/**
 * @brief Test duplicate username registration is rejected
 * @details Registers a user and then attempts to register again with the same
 *          username, verifying that the duplicate is rejected due to UNIQUE constraint
 */
TEST_F(PasswordSQLTest, RegisterUser_Duplicate_ReturnsFalse)
{
    EXPECT_TRUE(instance_->RegisterUser("bob", "pass456"));
    // Duplicate username should fail because username is UNIQUE
    EXPECT_FALSE(instance_->RegisterUser("bob", "other_pass"));
}

/**
 * @brief Test registration with empty username is rejected
 * @details Verifies that attempting to register a user with an empty username
 *          string returns false
 */
TEST_F(PasswordSQLTest, RegisterUser_EmptyUsername_ReturnsFalse)
{
    EXPECT_FALSE(instance_->RegisterUser("", "pass123"));
}

/**
 * @brief Test registration with empty password is rejected
 * @details Verifies that attempting to register a user with an empty password
 *          string returns false
 */
TEST_F(PasswordSQLTest, RegisterUser_EmptyPassword_ReturnsFalse)
{
    EXPECT_FALSE(instance_->RegisterUser("charlie", ""));
}

/**
 * @brief Test successful user authentication
 * @details Registers a user and then authenticates with the correct password,
 *          verifying that authentication succeeds
 */
TEST_F(PasswordSQLTest, AuthenticateUser_Success)
{
    instance_->RegisterUser("dave", "secret123");
    EXPECT_TRUE(instance_->AuthenticateUser("dave", "secret123"));
}

/**
 * @brief Test authentication with wrong password is rejected
 * @details Registers a user and then attempts authentication with an incorrect
 *          password, verifying that it returns false
 */
TEST_F(PasswordSQLTest, AuthenticateUser_WrongPassword)
{
    instance_->RegisterUser("eve", "real_pass");
    EXPECT_FALSE(instance_->AuthenticateUser("eve", "wrong_pass"));
}

/**
 * @brief Test authentication of a non-existent user returns false
 * @details Attempts to authenticate a username that has never been registered,
 *          verifying that the operation returns false
 */
TEST_F(PasswordSQLTest, AuthenticateUser_NonExistentUser)
{
    EXPECT_FALSE(instance_->AuthenticateUser("nonexistent", "pass123"));
}

/**
 * @brief Test authentication with empty username or password is rejected
 * @details Verifies that authentication returns false when either the username
 *          or the password is an empty string
 */
TEST_F(PasswordSQLTest, AuthenticateUser_EmptyInput)
{
    EXPECT_FALSE(instance_->AuthenticateUser("", "pass"));
    EXPECT_FALSE(instance_->AuthenticateUser("user", ""));
}

/**
 * @brief Test successful password change
 * @details Registers a user, changes the password with the correct old password,
 *          and verifies that the old password no longer works while the new one does
 */
TEST_F(PasswordSQLTest, ChangePassword_Success)
{
    instance_->RegisterUser("frank", "old_pass");
    EXPECT_TRUE(instance_->ChangePassword("frank", "old_pass", "new_pass"));
    // Old password should no longer work
    EXPECT_FALSE(instance_->AuthenticateUser("frank", "old_pass"));
    // New password should work
    EXPECT_TRUE(instance_->AuthenticateUser("frank", "new_pass"));
}

/**
 * @brief Test password change with wrong old password is rejected
 * @details Attempts to change password using an incorrect old password,
 *          verifying that the operation returns false and the original password
 *          remains valid
 */
TEST_F(PasswordSQLTest, ChangePassword_WrongOldPassword)
{
    instance_->RegisterUser("grace", "correct_pass");
    EXPECT_FALSE(instance_->ChangePassword("grace", "wrong_pass", "new_pass"));
    // Original password must remain valid
    EXPECT_TRUE(instance_->AuthenticateUser("grace", "correct_pass"));
}

/**
 * @brief Test password change for a non-existent user returns false
 * @details Attempts to change the password for a username that has never been
 *          registered, verifying that the operation returns false
 */
TEST_F(PasswordSQLTest, ChangePassword_NonExistentUser)
{
    EXPECT_FALSE(instance_->ChangePassword("ghost", "old", "new"));
}

/**
 * @brief Test successful password reset
 * @details Registers a user, resets the password without requiring the old one,
 *          and verifies that the old password no longer works while the new one does
 */
TEST_F(PasswordSQLTest, ResetPassword_Success)
{
    instance_->RegisterUser("heidi", "original_pass");
    EXPECT_TRUE(instance_->ResetPassword("heidi", "reset_pass"));
    // Old password should no longer work
    EXPECT_FALSE(instance_->AuthenticateUser("heidi", "original_pass"));
    // New password should work
    EXPECT_TRUE(instance_->AuthenticateUser("heidi", "reset_pass"));
}

/**
 * @brief Test password reset for a non-existent user returns false
 * @details Attempts to reset the password for a username that has never been
 *          registered, verifying that the operation returns false
 */
TEST_F(PasswordSQLTest, ResetPassword_NonExistentUser)
{
    EXPECT_FALSE(instance_->ResetPassword("ghost", "new_pass"));
}

/**
 * @brief Test successful user deletion
 * @details Registers a user, verifies they exist, deletes them, and confirms
 *          that the user no longer exists in the database
 */
TEST_F(PasswordSQLTest, DeleteUser_Success)
{
    instance_->RegisterUser("ivan", "delete_me");
    EXPECT_TRUE(instance_->UserExists("ivan"));
    EXPECT_TRUE(instance_->DeleteUser("ivan"));
    EXPECT_FALSE(instance_->UserExists("ivan"));
}

/**
 * @brief Test deletion of a non-existent user returns false
 * @details Attempts to delete a username that has never been registered,
 *          verifying that the operation returns false
 */
TEST_F(PasswordSQLTest, DeleteUser_NonExistent)
{
    EXPECT_FALSE(instance_->DeleteUser("nobody"));
}

/**
 * @brief Test deletion with an empty username returns false
 * @details Verifies that attempting to delete a user with an empty username
 *          string returns false
 */
TEST_F(PasswordSQLTest, DeleteUser_EmptyUsername)
{
    EXPECT_FALSE(instance_->DeleteUser(""));
}

/**
 * @brief Test that an existing user is detected
 * @details Registers a user and then verifies that UserExists returns true
 *          for the registered username
 */
TEST_F(PasswordSQLTest, UserExists_True)
{
    instance_->RegisterUser("judy", "exists_pass");
    EXPECT_TRUE(instance_->UserExists("judy"));
}

/**
 * @brief Test that a non-existent user is not detected
 * @details Verifies that UserExists returns false for a username that has
 *          never been registered
 */
TEST_F(PasswordSQLTest, UserExists_False)
{
    EXPECT_FALSE(instance_->UserExists("no_one_here"));
}

/**
 * @brief Test UserExists with an empty username returns false
 * @details Verifies that querying user existence with an empty username
 *          string returns false
 */
TEST_F(PasswordSQLTest, UserExists_Empty)
{
    EXPECT_FALSE(instance_->UserExists(""));
}

/**
 * @brief Test retrieving an existing user by username
 * @details Registers a user and then retrieves them by username, verifying
 *          that the returned username matches
 */
TEST_F(PasswordSQLTest, GetUser_Found)
{
    instance_->RegisterUser("karl", "any_pass");
    const auto result = instance_->GetUser("karl");
    EXPECT_EQ(result, "karl");
}

/**
 * @brief Test retrieving a non-existent user returns empty string
 * @details Attempts to retrieve a username that has never been registered,
 *          verifying that the result is an empty string
 */
TEST_F(PasswordSQLTest, GetUser_NotFound)
{
    EXPECT_TRUE(instance_->GetUser("missing").empty());
}

/**
 * @brief Test GetUser with an empty username returns empty string
 * @details Verifies that retrieving a user with an empty username string
 *          returns an empty result
 */
TEST_F(PasswordSQLTest, GetUser_EmptyInput)
{
    EXPECT_TRUE(instance_->GetUser("").empty());
}

/**
 * @brief Test GetAllUsers returns empty list when no users exist
 * @details Verifies that querying all users on a clean database returns
 *          an empty result set
 */
TEST_F(PasswordSQLTest, GetAllUsers_Empty)
{
    EXPECT_TRUE(instance_->GetAllUsers().empty());
}

/**
 * @brief Test GetAllUsers returns all registered users in alphabetical order
 * @details Registers multiple users and verifies that GetAllUsers returns all
 *          of them, sorted alphabetically by username
 */
TEST_F(PasswordSQLTest, GetAllUsers_Multiple)
{
    instance_->RegisterUser("mallory", "pass1");
    instance_->RegisterUser("nancy", "pass2");
    instance_->RegisterUser("oscar", "pass3");

    const auto users = instance_->GetAllUsers();
    EXPECT_EQ(users.size(), 3);

    // Results should be sorted alphabetically
    EXPECT_EQ(users[0], "mallory");
    EXPECT_EQ(users[1], "nancy");
    EXPECT_EQ(users[2], "oscar");
}
