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

TEST_F(PasswordSQLTest, RegisterUser_Success)
{
    EXPECT_TRUE(instance_->RegisterUser("alice", "pass123"));
    EXPECT_TRUE(instance_->UserExists("alice"));
}

TEST_F(PasswordSQLTest, RegisterUser_Duplicate_ReturnsFalse)
{
    EXPECT_TRUE(instance_->RegisterUser("bob", "pass456"));
    // Duplicate username should fail because username is UNIQUE
    EXPECT_FALSE(instance_->RegisterUser("bob", "other_pass"));
}

TEST_F(PasswordSQLTest, RegisterUser_EmptyUsername_ReturnsFalse)
{
    EXPECT_FALSE(instance_->RegisterUser("", "pass123"));
}

TEST_F(PasswordSQLTest, RegisterUser_EmptyPassword_ReturnsFalse)
{
    EXPECT_FALSE(instance_->RegisterUser("charlie", ""));
}

TEST_F(PasswordSQLTest, AuthenticateUser_Success)
{
    instance_->RegisterUser("dave", "secret123");
    EXPECT_TRUE(instance_->AuthenticateUser("dave", "secret123"));
}

TEST_F(PasswordSQLTest, AuthenticateUser_WrongPassword)
{
    instance_->RegisterUser("eve", "real_pass");
    EXPECT_FALSE(instance_->AuthenticateUser("eve", "wrong_pass"));
}

TEST_F(PasswordSQLTest, AuthenticateUser_NonExistentUser)
{
    EXPECT_FALSE(instance_->AuthenticateUser("nonexistent", "pass123"));
}

TEST_F(PasswordSQLTest, AuthenticateUser_EmptyInput)
{
    EXPECT_FALSE(instance_->AuthenticateUser("", "pass"));
    EXPECT_FALSE(instance_->AuthenticateUser("user", ""));
}

TEST_F(PasswordSQLTest, ChangePassword_Success)
{
    instance_->RegisterUser("frank", "old_pass");
    EXPECT_TRUE(instance_->ChangePassword("frank", "old_pass", "new_pass"));
    // Old password should no longer work
    EXPECT_FALSE(instance_->AuthenticateUser("frank", "old_pass"));
    // New password should work
    EXPECT_TRUE(instance_->AuthenticateUser("frank", "new_pass"));
}

TEST_F(PasswordSQLTest, ChangePassword_WrongOldPassword)
{
    instance_->RegisterUser("grace", "correct_pass");
    EXPECT_FALSE(instance_->ChangePassword("grace", "wrong_pass", "new_pass"));
    // Original password must remain valid
    EXPECT_TRUE(instance_->AuthenticateUser("grace", "correct_pass"));
}

TEST_F(PasswordSQLTest, ChangePassword_NonExistentUser)
{
    EXPECT_FALSE(instance_->ChangePassword("ghost", "old", "new"));
}

TEST_F(PasswordSQLTest, ResetPassword_Success)
{
    instance_->RegisterUser("heidi", "original_pass");
    EXPECT_TRUE(instance_->ResetPassword("heidi", "reset_pass"));
    // Old password should no longer work
    EXPECT_FALSE(instance_->AuthenticateUser("heidi", "original_pass"));
    // New password should work
    EXPECT_TRUE(instance_->AuthenticateUser("heidi", "reset_pass"));
}

TEST_F(PasswordSQLTest, ResetPassword_NonExistentUser)
{
    EXPECT_FALSE(instance_->ResetPassword("ghost", "new_pass"));
}

TEST_F(PasswordSQLTest, DeleteUser_Success)
{
    instance_->RegisterUser("ivan", "delete_me");
    EXPECT_TRUE(instance_->UserExists("ivan"));
    EXPECT_TRUE(instance_->DeleteUser("ivan"));
    EXPECT_FALSE(instance_->UserExists("ivan"));
}

TEST_F(PasswordSQLTest, DeleteUser_NonExistent)
{
    EXPECT_FALSE(instance_->DeleteUser("nobody"));
}

TEST_F(PasswordSQLTest, DeleteUser_EmptyUsername)
{
    EXPECT_FALSE(instance_->DeleteUser(""));
}

TEST_F(PasswordSQLTest, UserExists_True)
{
    instance_->RegisterUser("judy", "exists_pass");
    EXPECT_TRUE(instance_->UserExists("judy"));
}

TEST_F(PasswordSQLTest, UserExists_False)
{
    EXPECT_FALSE(instance_->UserExists("no_one_here"));
}

TEST_F(PasswordSQLTest, UserExists_Empty)
{
    EXPECT_FALSE(instance_->UserExists(""));
}

TEST_F(PasswordSQLTest, GetUser_Found)
{
    instance_->RegisterUser("karl", "any_pass");
    const auto result = instance_->GetUser("karl");
    EXPECT_EQ(result, "karl");
}

TEST_F(PasswordSQLTest, GetUser_NotFound)
{
    EXPECT_TRUE(instance_->GetUser("missing").empty());
}

TEST_F(PasswordSQLTest, GetUser_EmptyInput)
{
    EXPECT_TRUE(instance_->GetUser("").empty());
}

TEST_F(PasswordSQLTest, GetAllUsers_Empty)
{
    EXPECT_TRUE(instance_->GetAllUsers().empty());
}

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
