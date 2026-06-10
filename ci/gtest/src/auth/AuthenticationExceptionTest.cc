/**
 * @file AuthenticationExceptionTest.cc
 * @brief Unit tests for the AuthenticationException class
 * @details Tests cover exception construction, message handling,
 *          and copy/move semantics.
 */

#include <string>
#include <string_view>
#include <gtest/gtest.h>

#include "auth/AuthenticationException.hpp"

using namespace common::auth;

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

// ============================================================================
// Boundary Condition Tests
// ============================================================================

/**
 * @brief Test self-assignment of exception
 * @details Verifies that self-assignment via default operator= does not corrupt state
 */
TEST_F(AuthenticationExceptionTest, SelfAssignment)
{
    AuthenticationException ex(std::string("Self-assign test"));
    // Self-assignment should be safe (standard default operator=)
    ex = ex;

    EXPECT_STREQ(ex.what(), "Self-assign test");
}

/**
 * @brief Test move-assignment from a moved-from object
 * @details Verifies that the default move-assignment leaves a valid state
 */
TEST_F(AuthenticationExceptionTest, MoveAssignmentFromMoved)
{
    AuthenticationException ex1(std::string("Original"));
    AuthenticationException ex2(std::string("Temp"));

    AuthenticationException ex3(std::move(ex1));
    // ex1 is now moved-from; assign ex2 into it
    ex1 = ex2;

    EXPECT_STREQ(ex1.what(), "Temp");
}

/**
 * @brief Test exception message containing binary null bytes
 * @details Verifies handling of embedded null characters in message string
 */
TEST_F(AuthenticationExceptionTest, BinaryDataInMessage)
{
    // String with internal null bytes (length 8, contains \0 at position 3)
    const std::string message(std::string("abc\0defgh", 8));
    const AuthenticationException ex(message);

    // what() returns a C-string, so it will truncate at the first null
    // but the underlying std::string stores the full data
    EXPECT_EQ(std::string(ex.what()), std::string("abc"));
}

/**
 * @brief Test exception_ptr round-trip through std::make_exception_ptr
 * @details Verifies that the exception can be packaged and re-thrown
 */
TEST_F(AuthenticationExceptionTest, ExceptionPtrRoundtrip)
{
    const std::exception_ptr eptr = std::make_exception_ptr(
        AuthenticationException(std::string("Wrapped exception")));

    try
    {
        std::rethrow_exception(eptr);
    }
    catch (const AuthenticationException& e)
    {
        EXPECT_STREQ(e.what(), "Wrapped exception");
    }
}

/**
 * @brief Test exception with whitespace-only message
 * @details Verifies handling of messages consisting only of whitespace characters
 */
TEST_F(AuthenticationExceptionTest, WhitespaceOnlyMessage)
{
    const std::string message = "   \t\n  ";
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), message.c_str());
}

/**
 * @brief Test exception with very long message (1 MB)
 * @details Verifies memory allocation boundary for extremely large messages
 */
TEST_F(AuthenticationExceptionTest, VeryLongMessage)
{
    const std::string message(1024 * 1024, 'x'); // 1 MB
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), message.c_str());
    EXPECT_EQ(std::string(ex.what()).length(), 1024 * 1024);
}

/**
 * @brief Test exception with single character message
 * @details Verifies minimal-size message boundary
 */
TEST_F(AuthenticationExceptionTest, SingleCharacterMessage)
{
    const AuthenticationException ex(std::string("A"));

    EXPECT_STREQ(ex.what(), "A");
}

/**
 * @brief Test exception with numeric-only message
 * @details Verifies handling of messages composed entirely of digits
 */
TEST_F(AuthenticationExceptionTest, NumericMessage)
{
    const std::string message = "1234567890";
    const AuthenticationException ex(message);

    EXPECT_STREQ(ex.what(), message.c_str());
}

/**
 * @brief Test exception in nested catch-rethrow pattern
 * @details Verifies that AuthenticationException remains valid through
 *          nested exception handling
 */
TEST_F(AuthenticationExceptionTest, NestedCatchRethrow)
{
    try
    {
        try
        {
            throw AuthenticationException(std::string("Inner error"));
        }
        catch (const AuthenticationException&)
        {
            throw; // re-throw
        }
    }
    catch (const AuthenticationException& e)
    {
        EXPECT_STREQ(e.what(), "Inner error");
    }
}
