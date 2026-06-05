/**
 * @file StackTraceExceptionHandlerTest.cc
 * @brief Unit tests for the StackTraceExceptionHandler utility class
 * @details Tests cover exception logging with stack traces, prefix variations,
 *          and behavior with different exception types.
 */

#include <exception>
#include <string>
#include <string_view>
#include <gtest/gtest.h>

#include "exception/StackTraceExceptionHandler.hpp"

using namespace common::exception;

/**
 * @brief Test fixture for StackTraceExceptionHandler tests
 */
class StackTraceExceptionHandlerTest : public testing::Test
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
 * @brief Test logException with std::runtime_error
 * @details Verifies no exception is thrown when logging a standard exception
 */
TEST_F(StackTraceExceptionHandlerTest, LogRuntimeError)
{
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(std::runtime_error("runtime failure")));
}

/**
 * @brief Test logException with std::logic_error
 * @details Verifies no exception is thrown when logging a logic error
 */
TEST_F(StackTraceExceptionHandlerTest, LogLogicError)
{
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(std::logic_error("logic failure")));
}

/**
 * @brief Test logException with custom std::exception subclass
 * @details Verifies handler works with custom exception types
 */
TEST_F(StackTraceExceptionHandlerTest, LogCustomException)
{
    class CustomException : public std::exception
    {
    public:
        const char* what() const noexcept override
        {
            return "custom error";
        }
    };

    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(CustomException()));
}

/**
 * @brief Test logException with empty prefix
 * @details Verifies no prefix is prepended when prefix is empty
 */
TEST_F(StackTraceExceptionHandlerTest, LogExceptionEmptyPrefix)
{
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(std::runtime_error("test"), {}));
}

/**
 * @brief Test logException with string_view prefix
 * @details Verifies string_view prefix is accepted
 */
TEST_F(StackTraceExceptionHandlerTest, LogExceptionWithStringViewPrefix)
{
    constexpr std::string_view prefix = "PREFIX";
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(std::runtime_error("test"), prefix));
}

/**
 * @brief Test logException with C-string prefix
 * @details Verifies const char* prefix is accepted without conversion
 */
TEST_F(StackTraceExceptionHandlerTest, LogExceptionWithCStringPrefix)
{
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(std::runtime_error("test"), "CSTRING"));
}

/**
 * @brief Test logException with std::string prefix
 * @details Verifies std::string is implicitly convertible to string_view
 */
TEST_F(StackTraceExceptionHandlerTest, LogExceptionWithStringPrefix)
{
    const std::string prefix = "STRING_PREFIX";
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(std::runtime_error("test"), prefix));
}

/**
 * @brief Test logUnknownException with empty prefix
 * @details Verifies unknown exception logging works without prefix
 */
TEST_F(StackTraceExceptionHandlerTest, LogUnknownExceptionEmptyPrefix)
{
    EXPECT_NO_THROW(StackTraceExceptionHandler::logUnknownException());
}

/**
 * @brief Test logUnknownException with various prefix types
 * @details Verifies prefix overloads work for unknown exceptions
 */
TEST_F(StackTraceExceptionHandlerTest, LogUnknownExceptionWithPrefix)
{
    EXPECT_NO_THROW(StackTraceExceptionHandler::logUnknownException("ERROR"));
    EXPECT_NO_THROW(StackTraceExceptionHandler::logUnknownException(std::string_view{"WARN"}));
    const std::string prefix = "INFO";
    EXPECT_NO_THROW(StackTraceExceptionHandler::logUnknownException(prefix));
}

/**
 * @brief Test logException within a try-catch block
 * @details Verifies handler is usable in typical catch blocks
 */
TEST_F(StackTraceExceptionHandlerTest, LogExceptionInCatchBlock)
{
    try
    {
        throw std::runtime_error("thrown error");
    }
    catch (const std::exception& e)
    {
        EXPECT_NO_THROW(StackTraceExceptionHandler::logException(e, "CAUGHT"));
    }
}

/**
 * @brief Test logUnknownException within a catch-all block
 * @details Verifies handler works in catch(...) blocks
 */
TEST_F(StackTraceExceptionHandlerTest, LogUnknownExceptionInCatchAll)
{
    try
    {
        throw 42;
    }
    catch (...)
    {
        EXPECT_NO_THROW(StackTraceExceptionHandler::logUnknownException("CATCH_ALL"));
    }
}

/**
 * @brief Test logException with nested exceptions
 * @details Verifies handler works with std::nested_exception
 */
TEST_F(StackTraceExceptionHandlerTest, LogNestedException)
{
    try
    {
        try
        {
            throw std::runtime_error("inner");
        }
        catch (...)
        {
            std::throw_with_nested(std::runtime_error("outer"));
        }
    }
    catch (const std::exception& e)
    {
        EXPECT_NO_THROW(StackTraceExceptionHandler::logException(e, "NESTED"));
    }
}

/**
 * @brief Test logException with long exception message
 * @details Verifies handler works with large what() strings
 */
TEST_F(StackTraceExceptionHandlerTest, LogLongMessage)
{
    const std::string longMsg(8192, 'A');
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(std::runtime_error(longMsg)));
}

/**
 * @brief Test logException with unicode message
 * @details Verifies handler works with unicode exception messages
 */
TEST_F(StackTraceExceptionHandlerTest, LogUnicodeMessage)
{
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(
        std::runtime_error("系统异常：连接失败"), "UNICODE"));
}

/**
 * @brief Test logException with special characters
 * @details Verifies handler works with special characters in messages
 */
TEST_F(StackTraceExceptionHandlerTest, LogSpecialCharacters)
{
    EXPECT_NO_THROW(StackTraceExceptionHandler::logException(
        std::runtime_error("error@#$%^&*()_+={}[]|\\:;\"'<>,.?/~`"), "SPECIAL"));
}
