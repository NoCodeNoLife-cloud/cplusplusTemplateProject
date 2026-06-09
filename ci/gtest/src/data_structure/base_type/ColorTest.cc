/**
 * @file ColorTest.cc
 * @brief Unit tests for the Color enum class
 * @details Tests cover enum values, integer conversion, and switch handling.
 */

#include <gtest/gtest.h>

#include "data_structure/base_type/Color.hpp"

using namespace common::data_structure;

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

TEST_F(ColorTest, EnumValues_Red)
{
    EXPECT_EQ(static_cast<int>(Color::Red), 0);
}

TEST_F(ColorTest, EnumValues_Black)
{
    EXPECT_EQ(static_cast<int>(Color::Black), 1);
}

TEST_F(ColorTest, EnumValues_Green)
{
    EXPECT_EQ(static_cast<int>(Color::Green), 2);
}

TEST_F(ColorTest, EnumValues_Blue)
{
    EXPECT_EQ(static_cast<int>(Color::Blue), 3);
}

TEST_F(ColorTest, EnumValues_White)
{
    EXPECT_EQ(static_cast<int>(Color::White), 4);
}

TEST_F(ColorTest, EnumValues_Yellow)
{
    EXPECT_EQ(static_cast<int>(Color::Yellow), 5);
}

TEST_F(ColorTest, EnumValues_Cyan)
{
    EXPECT_EQ(static_cast<int>(Color::Cyan), 6);
}

TEST_F(ColorTest, EnumValues_Magenta)
{
    EXPECT_EQ(static_cast<int>(Color::Magenta), 7);
}

TEST_F(ColorTest, EnumSize_EightValues)
{
    EXPECT_EQ(static_cast<int>(Color::Magenta) + 1, 8);
}

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
