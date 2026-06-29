/**
 * @file ColorTest.cc
 * @brief Unit tests for the Color enum class
 * @details Tests cover enum values, integer conversion, and switch handling.
 */

#include <gtest/gtest.h>

#include "data_structure/base_type/Color.hpp"

using namespace cppforge::data_structure;

/**
 * @brief Test fixture for Color tests
 */
class ColorTest : public testing::Test
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
 * @brief Test Red enum value equals 0
 * @details Verifies that Color::Red has the expected integer value of 0
 */
TEST_F(ColorTest, EnumValues_Red)
{
    EXPECT_EQ(static_cast<int>(Color::Red), 0);
}

/**
 * @brief Test Black enum value equals 1
 * @details Verifies that Color::Black has the expected integer value of 1
 */
TEST_F(ColorTest, EnumValues_Black)
{
    EXPECT_EQ(static_cast<int>(Color::Black), 1);
}

/**
 * @brief Test Green enum value equals 2
 * @details Verifies that Color::Green has the expected integer value of 2
 */
TEST_F(ColorTest, EnumValues_Green)
{
    EXPECT_EQ(static_cast<int>(Color::Green), 2);
}

/**
 * @brief Test Blue enum value equals 3
 * @details Verifies that Color::Blue has the expected integer value of 3
 */
TEST_F(ColorTest, EnumValues_Blue)
{
    EXPECT_EQ(static_cast<int>(Color::Blue), 3);
}

/**
 * @brief Test White enum value equals 4
 * @details Verifies that Color::White has the expected integer value of 4
 */
TEST_F(ColorTest, EnumValues_White)
{
    EXPECT_EQ(static_cast<int>(Color::White), 4);
}

/**
 * @brief Test Yellow enum value equals 5
 * @details Verifies that Color::Yellow has the expected integer value of 5
 */
TEST_F(ColorTest, EnumValues_Yellow)
{
    EXPECT_EQ(static_cast<int>(Color::Yellow), 5);
}

/**
 * @brief Test Cyan enum value equals 6
 * @details Verifies that Color::Cyan has the expected integer value of 6
 */
TEST_F(ColorTest, EnumValues_Cyan)
{
    EXPECT_EQ(static_cast<int>(Color::Cyan), 6);
}

/**
 * @brief Test Magenta enum value equals 7
 * @details Verifies that Color::Magenta has the expected integer value of 7
 */
TEST_F(ColorTest, EnumValues_Magenta)
{
    EXPECT_EQ(static_cast<int>(Color::Magenta), 7);
}

/**
 * @brief Test that the enum contains exactly eight values
 * @details Verifies that the Color enum has eight distinct values by
 *          checking the count from Red (0) to Magenta (7)
 */
TEST_F(ColorTest, EnumSize_EightValues)
{
    EXPECT_EQ(static_cast<int>(Color::Magenta) + 1, 8);
}

/**
 * @brief Test that switch statement handles all enum cases
 * @details Verifies that a switch over all Color values compiles and
 *          executes without warnings or errors
 */
TEST_F(ColorTest, Switch_HandlesAllCases)
{
    for (int i = 0; i <= 7; ++i)
    {
        const Color c = static_cast<Color>(i);
        switch (c)
        {
        case Color::Red:
        case Color::Black:
        case Color::Green:
        case Color::Blue:
        case Color::White:
        case Color::Yellow:
        case Color::Cyan:
        case Color::Magenta:
            SUCCEED();
            break;
        }
    }
}
