/**
 * @file AuthenticationExceptionTest.cc
 * @brief Unit tests for the AuthenticationException class
 * @details Tests cover exception construction, message handling,
 *          and copy/move semantics.
 */

#include <string>
#include <string_view>
#include <gtest/gtest.h>

#include "exception/AuthenticationException.hpp"

using namespace common::exception;

/**
 * @brief Test fixture for AuthenticationExceptionTest tests
 */
class AuthenticationExceptionTest : public testing::Test
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
 * @brief Test exception construction with string message
 * @details Verifies that exception can be created with std::string message
 */
TEST_F(AuthenticationExceptionTest, ConstructionWithString)
{
    const std::string message = "Authentication failed: invalid credentials";
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), message.c_str());
}

/**
 * @brief Test exception construction with string_view message
 * @details Verifies that exception can be created with std::string_view
 */
TEST_F(AuthenticationExceptionTest, ConstructionWithStringView)
{
    const std::string_view message = "Invalid username or password";
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), std::string(message).c_str());
}

/**
 * @brief Test exception construction with C-string
 * @details Verifies that exception can be created with const char*
 */
TEST_F(AuthenticationExceptionTest, ConstructionWithCString)
{
    const AuthenticationException ex(std::string("Access denied"));

    EXPECT_STREQ(ex.what(), "Access denied");
}

/**
 * @brief Test exception with empty message
 * @details Verifies that exception handles empty messages correctly
 */
TEST_F(AuthenticationExceptionTest, EmptyMessage)
{
    const AuthenticationException ex(std::string(""));

    EXPECT_STREQ(ex.what(), "");
}

/**
 * @brief Test exception with long message
 * @details Verifies that exception handles long messages correctly
 */
TEST_F(AuthenticationExceptionTest, LongMessage)
{
    const std::string message(10000, 'x'); // 10KB message
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), message.c_str());
    EXPECT_EQ(std::string(ex.what()).length(), 10000);
}

/**
 * @brief Test exception with special characters in message
 * @details Verifies that exception handles special characters correctly
 */
TEST_F(AuthenticationExceptionTest, SpecialCharactersInMessage)
{
    const std::string message = "Error: user@test.com - invalid @#$%^&*()";
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), message.c_str());
}

/**
 * @brief Test exception with Unicode message
 * @details Verifies that exception handles Unicode characters correctly
 */
TEST_F(AuthenticationExceptionTest, UnicodeMessage)
{
    const std::string message = "认证失败：用户名或密码错误 🔐";
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), message.c_str());
}

/**
 * @brief Test exception copy constructor
 * @details Verifies that exception can be copied correctly
 */
TEST_F(AuthenticationExceptionTest, CopyConstructor)
{
    const AuthenticationException ex1(std::string("Original error message"));
    const AuthenticationException ex2(ex1);

    EXPECT_STREQ(ex1.what(), ex2.what());
    EXPECT_STREQ(ex2.what(), "Original error message");
}

/**
 * @brief Test exception move constructor
 * @details Verifies that exception can be moved efficiently
 */
TEST_F(AuthenticationExceptionTest, MoveConstructor)
{
    AuthenticationException ex1(std::string("Moveable error"));
    const AuthenticationException ex2(std::move(ex1));

    EXPECT_STREQ(ex2.what(), "Moveable error");
}

/**
 * @brief Test exception assignment operator
 * @details Verifies that exception assignment works correctly
 */
TEST_F(AuthenticationExceptionTest, AssignmentOperator)
{
    AuthenticationException ex1(std::string("First error"));
    const AuthenticationException ex2(std::string("Second error"));

    ex1 = ex2;

    EXPECT_STREQ(ex1.what(), "Second error");
}

/**
 * @brief Test exception move assignment operator
 * @details Verifies that move assignment works correctly
 */
TEST_F(AuthenticationExceptionTest, MoveAssignmentOperator)
{
    AuthenticationException ex1(std::string("First error"));
    AuthenticationException ex2(std::string("Second error"));

    ex1 = std::move(ex2);

    EXPECT_STREQ(ex1.what(), "Second error");
}

/**
 * @brief Test exception inherits from std::runtime_error
 * @details Verifies proper inheritance hierarchy
 */
TEST_F(AuthenticationExceptionTest, InheritanceFromRuntimeError)
{
    const AuthenticationException ex(std::string("Test error"));

    // Should be catchable as std::runtime_error
    try
    {
        throw ex;
    }
    catch (const std::runtime_error& e)
    {
        EXPECT_STREQ(e.what(), "Test error");
    }
}

/**
 * @brief Test exception inherits from std::exception
 * @details Verifies base exception compatibility
 */
TEST_F(AuthenticationExceptionTest, InheritanceFromException)
{
    const AuthenticationException ex(std::string("Base test"));

    // Should be catchable as std::exception
    try
    {
        throw ex;
    }
    catch (const std::exception& e)
    {
        EXPECT_STREQ(e.what(), "Base test");
    }
}

/**
 * @brief Test exception can be thrown and caught
 * @details Verifies basic exception throwing mechanism
 */
TEST_F(AuthenticationExceptionTest, ThrowAndCatch)
{
    try
    {
        throw AuthenticationException(std::string("Thrown error"));
    }
    catch (const AuthenticationException& e)
    {
        EXPECT_STREQ(e.what(), "Thrown error");
    }
}

/**
 * @brief Test exception with multiline message
 * @details Verifies that exception handles multiline messages
 */
TEST_F(AuthenticationExceptionTest, MultilineMessage)
{
    const std::string message = "Line 1\nLine 2\nLine 3";
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), message.c_str());
}

/**
 * @brief Test exception message persistence
 * @details Verifies that exception message persists throughout lifetime
 */
TEST_F(AuthenticationExceptionTest, MessagePersistence)
{
    const auto ex = std::make_unique<AuthenticationException>(std::string("Persistent message"));

    EXPECT_STREQ(ex->what(), "Persistent message");

    // Message should still be valid after some operations
    const std::string msg = ex->what();
    EXPECT_EQ(msg, "Persistent message");
}

/**
 * @brief Test multiple exceptions with different messages
 * @details Verifies that multiple exceptions maintain separate messages
 */
TEST_F(AuthenticationExceptionTest, MultipleExceptions)
{
    const AuthenticationException ex1(std::string("Error 1"));
    const AuthenticationException ex2(std::string("Error 2"));
    const AuthenticationException ex3(std::string("Error 3"));

    EXPECT_STREQ(ex1.what(), "Error 1");
    EXPECT_STREQ(ex2.what(), "Error 2");
    EXPECT_STREQ(ex3.what(), "Error 3");
}

/**
 * @brief Test exception in polymorphic context
 * @details Verifies that exception works correctly in polymorphic scenarios
 */
TEST_F(AuthenticationExceptionTest, PolymorphicContext)
{
    const std::exception* ex_ptr = new AuthenticationException(std::string("Polymorphic error"));

    EXPECT_STREQ(ex_ptr->what(), "Polymorphic error");

    delete ex_ptr;
}

/**
 * @brief Test exception with various authentication error messages
 * @details Verifies common authentication error scenarios
 */
TEST_F(AuthenticationExceptionTest, CommonAuthErrorMessages)
{
    const std::vector<std::string> messages = {
        "Invalid username",
        "Invalid password",
        "Account locked",
        "Password expired",
        "Too many failed attempts",
        "User not found",
        "Authentication timeout",
        "Invalid token",
        "Session expired",
        "Access denied"
    };

    for (const auto& msg : messages)
    {
        const AuthenticationException ex(msg);
        EXPECT_STREQ(ex.what(), msg.c_str());
    }
}
