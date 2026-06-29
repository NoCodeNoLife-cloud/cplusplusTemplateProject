/**
 * @file PrefixFormatterTest.cc
 * @brief Unit tests for the PrefixFormatter class
 * @details Tests cover class construction, copy/move semantics,
 *          and log prefix format output via MyPrefixFormatter.
 */

#include <filesystem>
#include <regex>
#include <string>
#include <gtest/gtest.h>
#include <glog/logging.h>

#include <cppforge/glog/formatter/PrefixFormatter.hpp>

using namespace cppforge::glog::formatter;

/**
 * @brief Verify prefix output matches expected format
 * @param result The output string from MyPrefixFormatter
 * @param severity Expected severity string (e.g., INFO, WARNING, ERROR)
 * @param filename Expected source file name
 * @param line Expected source line number
 */
static void verifyFormat(const std::string& result,
                         const std::string& severity,
                         const std::string& filename,
                         int line)
{
    // Format: [SEVERITY] [YYYYMMDD HH:MM:SS.ffffff] [ TTTTT] [file:line]
    const std::string pattern =
        R"(\[)" + severity + R"(\] \[\d{4}\d{2}\d{2} \d{2}:\d{2}:\d{2}\.\d{6}\] \[\s*\d+\] \[)" +
        std::regex_replace(filename, std::regex(R"([.])"), R"(\.)") +
        ":" + std::to_string(line) + R"(\] )";

    EXPECT_TRUE(std::regex_match(result, std::regex(pattern)))
        << "Result: [" << result << "] does not match pattern: [" << pattern << "]";
}

/**
 * @brief Test fixture for PrefixFormatter tests
 */
class PrefixFormatterTest : public testing::Test
{
protected:
    std::string log_dir_;

    void SetUp() override
    {
        log_dir_ = (std::filesystem::temp_directory_path() / "PrefixFormatterTest").string();
        std::filesystem::create_directories(log_dir_);
        google::InitGoogleLogging("prefix_formatter_test");
        FLAGS_log_dir = log_dir_;
        FLAGS_logtostderr = false;
        FLAGS_minloglevel = 0;
    }

    void TearDown() override
    {
        google::ShutdownGoogleLogging();
        std::error_code ec;
        std::filesystem::remove_all(log_dir_, ec);
    }
};

/**
 * @brief Test default construction
 */
TEST_F(PrefixFormatterTest, DefaultConstruction)
{
    EXPECT_NO_THROW(PrefixFormatter());
}

/**
 * @brief Test copy construction
 */
TEST_F(PrefixFormatterTest, CopyConstruction)
{
    constexpr PrefixFormatter fmt1;
    EXPECT_NO_THROW((void)PrefixFormatter(fmt1));
}

/**
 * @brief Test copy assignment
 */
TEST_F(PrefixFormatterTest, CopyAssignment)
{
    constexpr PrefixFormatter fmt1;
    EXPECT_NO_THROW((void)(PrefixFormatter() = fmt1));
}

/**
 * @brief Test move construction
 */
TEST_F(PrefixFormatterTest, MoveConstruction)
{
    PrefixFormatter fmt1;
    EXPECT_NO_THROW(PrefixFormatter(static_cast<PrefixFormatter&&>(fmt1)));
}

/**
 * @brief Test move assignment
 */
TEST_F(PrefixFormatterTest, MoveAssignment)
{
    PrefixFormatter fmt1;
    EXPECT_NO_THROW((void)(PrefixFormatter() = static_cast<PrefixFormatter&&>(fmt1)));
}

/**
 * @brief Test constexpr default constructor
 */
TEST_F(PrefixFormatterTest, ConstexprConstructor)
{
    constexpr PrefixFormatter fmt;
    (void)fmt;
}

/**
 * @brief Test MyPrefixFormatter produces expected format for INFO
 */
TEST_F(PrefixFormatterTest, FormatOutputInfo)
{
    const google::LogMessage msg("info_test.cc", 42, google::GLOG_INFO);
    std::ostringstream oss;
    PrefixFormatter::MyPrefixFormatter(oss, msg, nullptr);

    verifyFormat(oss.str(), "INFO", "info_test.cc", 42);
}

/**
 * @brief Test MyPrefixFormatter with WARNING severity
 */
TEST_F(PrefixFormatterTest, FormatOutputWarning)
{
    const google::LogMessage msg("warning_test.cc", 99, google::GLOG_WARNING);
    std::ostringstream oss;
    PrefixFormatter::MyPrefixFormatter(oss, msg, nullptr);

    verifyFormat(oss.str(), "WARNING", "warning_test.cc", 99);
}

/**
 * @brief Test MyPrefixFormatter with ERROR severity
 */
TEST_F(PrefixFormatterTest, FormatOutputError)
{
    const google::LogMessage msg("error_test.cc", 77, google::GLOG_ERROR);
    std::ostringstream oss;
    PrefixFormatter::MyPrefixFormatter(oss, msg, nullptr);

    verifyFormat(oss.str(), "ERROR", "error_test.cc", 77);
}

/**
 * @brief Test format preserves the original stream fill character
 */
TEST_F(PrefixFormatterTest, PreservesStreamFill)
{
    const google::LogMessage msg("fill_test.cc", 1, google::GLOG_INFO);
    std::ostringstream oss;
    oss.fill('*');
    PrefixFormatter::MyPrefixFormatter(oss, msg, nullptr);

    EXPECT_EQ(oss.fill(), '*');
}

/**
 * @brief Test format appends content without clearing existing stream content
 */
TEST_F(PrefixFormatterTest, AppendsToStream)
{
    const google::LogMessage msg("append_test.cc", 2, google::GLOG_INFO);
    std::ostringstream oss;
    oss << "PREFIX: ";
    PrefixFormatter::MyPrefixFormatter(oss, msg, nullptr);

    const std::string result = oss.str();
    EXPECT_TRUE(result.starts_with("PREFIX: "));
    verifyFormat(result.substr(8), "INFO", "append_test.cc", 2);
}

/**
 * @brief Test that format handles no trailing newline
 */
TEST_F(PrefixFormatterTest, NoTrailingNewline)
{
    const google::LogMessage msg("newline_test.cc", 10, google::GLOG_INFO);
    std::ostringstream oss;
    PrefixFormatter::MyPrefixFormatter(oss, msg, nullptr);

    const std::string result = oss.str();
    EXPECT_FALSE(result.ends_with('\n'));
}
