/**
 * @file PasswordPolicyTest.cc
 * @brief Unit tests for the PasswordPolicy class
 * @details Tests cover password validation rules, policy configuration,
 *          and edge cases for password requirements.
 */

#include <gtest/gtest.h>

#include "auth/PasswordPolicy.hpp"

using namespace cppforge::auth;

/**
 * @brief Test fixture for PasswordPolicy tests
 */
class PasswordPolicyTest : public testing::Test
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
 * @brief Test default password policy configuration
 * @details Verifies that default policy has expected values
 */
TEST_F(PasswordPolicyTest, DefaultConfiguration)
{
    const PasswordPolicy policy;

    EXPECT_EQ(policy.min_length(), 8);
    EXPECT_EQ(policy.max_length(), 64);
    EXPECT_TRUE(policy.require_uppercase());
    EXPECT_TRUE(policy.require_lowercase());
    EXPECT_TRUE(policy.require_digits());
    EXPECT_TRUE(policy.require_special());
    EXPECT_EQ(policy.max_login_attempts(), 5);
}

/**
 * @brief Test custom password policy configuration
 * @details Verifies that custom parameters are properly set
 */
TEST_F(PasswordPolicyTest, CustomConfiguration)
{
    const PasswordPolicy policy(12, 128, false, true, false, false, 3);

    EXPECT_EQ(policy.min_length(), 12);
    EXPECT_EQ(policy.max_length(), 128);
    EXPECT_FALSE(policy.require_uppercase());
    EXPECT_TRUE(policy.require_lowercase());
    EXPECT_FALSE(policy.require_digits());
    EXPECT_FALSE(policy.require_special());
    EXPECT_EQ(policy.max_login_attempts(), 3);
}

/**
 * @brief Test valid password with all requirements
 * @details Verifies that a password meeting all default requirements passes validation
 */
TEST_F(PasswordPolicyTest, Validate_ValidPassword_AllRequirements)
{
    const PasswordPolicy policy;

    // Password with uppercase, lowercase, digit, and special character
    EXPECT_TRUE(policy.validate("Secure@123"));
    EXPECT_TRUE(policy.validate("P@ssw0rd!"));
    EXPECT_TRUE(policy.validate("Str0ng#Pass"));
}

/**
 * @brief Test password too short
 * @details Verifies that passwords below minimum length fail validation
 */
TEST_F(PasswordPolicyTest, Validate_PasswordTooShort)
{
    const PasswordPolicy policy;

    EXPECT_FALSE(policy.validate("Ab1!")); // 4 chars
    EXPECT_FALSE(policy.validate("Short@1")); // 7 chars
    EXPECT_TRUE(policy.validate("Valid@12")); // 8 chars (minimum)
}

/**
 * @brief Test password too long
 * @details Verifies that passwords exceeding maximum length fail validation
 */
TEST_F(PasswordPolicyTest, Validate_PasswordTooLong)
{
    const PasswordPolicy policy(8, 20);

    EXPECT_TRUE(policy.validate("Valid@12345678901")); // 15 chars
    EXPECT_FALSE(policy.validate("ThisPasswordIsWayTooLong@123")); // 26 chars > 20
}

/**
 * @brief Test password missing uppercase
 * @details Verifies that passwords without uppercase letters fail when required
 */
TEST_F(PasswordPolicyTest, Validate_MissingUppercase)
{
    const PasswordPolicy policy;

    EXPECT_FALSE(policy.validate("password@123")); // No uppercase
    EXPECT_TRUE(policy.validate("Password@123")); // Has uppercase
}

/**
 * @brief Test password missing lowercase
 * @details Verifies that passwords without lowercase letters fail when required
 */
TEST_F(PasswordPolicyTest, Validate_MissingLowercase)
{
    const PasswordPolicy policy;

    EXPECT_FALSE(policy.validate("PASSWORD@123")); // No lowercase
    EXPECT_TRUE(policy.validate("Password@123")); // Has lowercase
}

/**
 * @brief Test password missing digits
 * @details Verifies that passwords without digits fail when required
 */
TEST_F(PasswordPolicyTest, Validate_MissingDigits)
{
    const PasswordPolicy policy;

    EXPECT_FALSE(policy.validate("Password@abc")); // No digits
    EXPECT_TRUE(policy.validate("Password@123")); // Has digits
}

/**
 * @brief Test password missing special characters
 * @details Verifies that passwords without special characters fail when required
 */
TEST_F(PasswordPolicyTest, Validate_MissingSpecial)
{
    const PasswordPolicy policy;

    EXPECT_FALSE(policy.validate("Password123")); // No special char
    EXPECT_TRUE(policy.validate("Password@123")); // Has special char
}

/**
 * @brief Test password validation with uppercase requirement disabled
 * @details Verifies that uppercase is not required when disabled
 */
TEST_F(PasswordPolicyTest, Validate_UppercaseNotRequired)
{
    PasswordPolicy policy;
    policy.set_require_uppercase(false);

    EXPECT_TRUE(policy.validate("password@123")); // No uppercase needed
    EXPECT_TRUE(policy.validate("Password@123")); // Still valid with uppercase
}

/**
 * @brief Test password validation with lowercase requirement disabled
 * @details Verifies that lowercase is not required when disabled
 */
TEST_F(PasswordPolicyTest, Validate_LowercaseNotRequired)
{
    PasswordPolicy policy;
    policy.set_require_lowercase(false);

    EXPECT_TRUE(policy.validate("PASSWORD@123")); // No lowercase needed
    EXPECT_TRUE(policy.validate("Password@123")); // Still valid with lowercase
}

/**
 * @brief Test password validation with digit requirement disabled
 * @details Verifies that digits are not required when disabled
 */
TEST_F(PasswordPolicyTest, Validate_DigitsNotRequired)
{
    PasswordPolicy policy;
    policy.set_require_digits(false);

    EXPECT_TRUE(policy.validate("Password@abc")); // No digits needed
    EXPECT_TRUE(policy.validate("Password@123")); // Still valid with digits
}

/**
 * @brief Test password validation with special character requirement disabled
 * @details Verifies that special characters are not required when disabled
 */
TEST_F(PasswordPolicyTest, Validate_SpecialNotRequired)
{
    PasswordPolicy policy;
    policy.set_require_special(false);

    EXPECT_TRUE(policy.validate("Password123")); // No special char needed
    EXPECT_TRUE(policy.validate("Password@123")); // Still valid with special char
}

/**
 * @brief Test empty password validation
 * @details Verifies that empty password fails validation
 */
TEST_F(PasswordPolicyTest, Validate_EmptyPassword)
{
    const PasswordPolicy policy;

    EXPECT_FALSE(policy.validate(""));
}

/**
 * @brief Test password with only spaces
 * @details Verifies that password with only whitespace fails validation
 */
TEST_F(PasswordPolicyTest, Validate_OnlySpaces)
{
    const PasswordPolicy policy;

    EXPECT_FALSE(policy.validate("        ")); // 8 spaces but no other requirements
}

/**
 * @brief Test password with special characters variety
 * @details Verifies that various special characters are accepted
 */
TEST_F(PasswordPolicyTest, Validate_SpecialCharactersVariety)
{
    const PasswordPolicy policy;

    EXPECT_TRUE(policy.validate("Pass@123")); // @
    EXPECT_TRUE(policy.validate("Pass#123")); // #
    EXPECT_TRUE(policy.validate("Pass$123")); // $
    EXPECT_TRUE(policy.validate("Pass%123")); // %
    EXPECT_TRUE(policy.validate("Pass^123")); // ^
    EXPECT_TRUE(policy.validate("Pass&123")); // &
    EXPECT_TRUE(policy.validate("Pass*123")); // *
}

/**
 * @brief Test password at exact boundary lengths
 * @details Verifies that passwords at min/max boundaries are accepted
 */
TEST_F(PasswordPolicyTest, Validate_BoundaryLengths)
{
    const PasswordPolicy policy(8, 16);

    EXPECT_TRUE(policy.validate("Ab1!xxxx")); // Exactly 8 chars (minimum)
    EXPECT_TRUE(policy.validate("Ab1!xxxxxxxx")); // 12 chars
    EXPECT_TRUE(policy.validate("Ab1!xxxxxxxxxx")); // 16 chars (exactly at max)
    EXPECT_FALSE(policy.validate("Ab1!xxx")); // 7 chars (too short)
    EXPECT_FALSE(policy.validate("Ab1!xxxxxxxxxxxxx")); // 17 chars (too long, exceeds max by 1)
}

/**
 * @brief Test setter methods for policy configuration
 * @details Verifies that all setter methods work correctly
 */
TEST_F(PasswordPolicyTest, SetterMethods)
{
    PasswordPolicy policy;

    policy.set_min_length(10);
    EXPECT_EQ(policy.min_length(), 10);

    policy.set_max_length(100);
    EXPECT_EQ(policy.max_length(), 100);

    policy.set_require_uppercase(false);
    EXPECT_FALSE(policy.require_uppercase());

    policy.set_require_lowercase(false);
    EXPECT_FALSE(policy.require_lowercase());

    policy.set_require_digits(false);
    EXPECT_FALSE(policy.require_digits());

    policy.set_require_special(false);
    EXPECT_FALSE(policy.require_special());
}

/**
 * @brief Test password with Unicode characters
 * @details Verifies that Unicode passwords are handled correctly
 */
TEST_F(PasswordPolicyTest, Validate_UnicodePassword)
{
    const PasswordPolicy policy;

    // Note: Unicode characters may count as multiple bytes
    // This test verifies the policy handles UTF-8 encoded strings
    EXPECT_TRUE(policy.validate("密码@Test123")); // Chinese + requirements
}

/**
 * @brief Test relaxed policy allows simpler passwords
 * @details Verifies that disabling all requirements allows simple passwords
 */
TEST_F(PasswordPolicyTest, Validate_RelaxedPolicy)
{
    PasswordPolicy policy;
    policy.set_require_uppercase(false);
    policy.set_require_lowercase(false);
    policy.set_require_digits(false);
    policy.set_require_special(false);
    policy.set_min_length(4);

    EXPECT_TRUE(policy.validate("test"));
    EXPECT_TRUE(policy.validate("1234"));
    EXPECT_TRUE(policy.validate("!!!!"));
}

/**
 * @brief Test strict policy requires complex passwords
 * @details Verifies that strict policy enforces all requirements
 */
TEST_F(PasswordPolicyTest, Validate_StrictPolicy)
{
    PasswordPolicy policy;
    policy.set_min_length(12);
    policy.set_require_uppercase(true);
    policy.set_require_lowercase(true);
    policy.set_require_digits(true);
    policy.set_require_special(true);

    EXPECT_FALSE(policy.validate("Short@123")); // Too short
    EXPECT_FALSE(policy.validate("nouppercase@123")); // No uppercase
    EXPECT_FALSE(policy.validate("NOLOWERCASE@123")); // No lowercase
    EXPECT_FALSE(policy.validate("NoDigits@abc")); // No digits
    EXPECT_FALSE(policy.validate("NoSpecial123")); // No special char

    EXPECT_TRUE(policy.validate("Complex@Pass123")); // Meets all
}

// ============================================================================
// Boundary Condition Tests
// ============================================================================

/**
 * @brief Test password validation when min_length > max_length
 * @details No password can satisfy this invalid configuration
 */
TEST_F(PasswordPolicyTest, Validate_MinGreaterThanMax)
{
    const PasswordPolicy policy(10, 5);

    EXPECT_FALSE(policy.validate("Abc@1xyz"));
    EXPECT_FALSE(policy.validate("Ab1!"));
    EXPECT_FALSE(policy.validate("AAAAAAAAAA"));
}

/**
 * @brief Test password with zero minimum length
 * @details Empty password should pass when min=0 and all requirements disabled
 */
TEST_F(PasswordPolicyTest, Validate_ZeroMinLength)
{
    PasswordPolicy policy(0, 64);
    policy.set_require_uppercase(false);
    policy.set_require_lowercase(false);
    policy.set_require_digits(false);
    policy.set_require_special(false);

    EXPECT_TRUE(policy.validate(""));
    EXPECT_TRUE(policy.validate("a"));
}

/**
 * @brief Test password with equal min and max length
 * @details Only passwords of exactly this length are accepted
 */
TEST_F(PasswordPolicyTest, Validate_MinEqualsMax)
{
    PasswordPolicy policy(5, 5);
    policy.set_require_uppercase(false);
    policy.set_require_lowercase(false);
    policy.set_require_digits(false);
    policy.set_require_special(false);

    EXPECT_TRUE(policy.validate("abcde"));  // exactly 5
    EXPECT_FALSE(policy.validate("abcd"));  // 4 - too short
    EXPECT_FALSE(policy.validate("abcdef")); // 6 - too long
}

/**
 * @brief Test that spaces count as special characters
 * @details is_special_char includes std::isspace, so a space meets the
 *          special character requirement
 */
TEST_F(PasswordPolicyTest, Validate_SpaceAsSpecial)
{
    PasswordPolicy policy(8, 64);
    policy.set_require_uppercase(true);
    policy.set_require_lowercase(true);
    policy.set_require_digits(true);
    policy.set_require_special(true);

    EXPECT_TRUE(policy.validate("Ab1 xyz!"));   // space as special, 8 chars
    EXPECT_TRUE(policy.validate("Ab1\txyz!"));   // tab as special, 8 chars
    EXPECT_TRUE(policy.validate("A b 1 x z!"));  // multiple spaces, 10 chars
}

/**
 * @brief Test password with only uppercase characters
 * @details Validates else-if chain: a single uppercase char is classified
 *          as uppercase only, not as special even if std::ispunct is false.
 */
TEST_F(PasswordPolicyTest, Validate_OnlyUppercase)
{
    PasswordPolicy policy(4, 64);
    policy.set_require_uppercase(true);
    policy.set_require_lowercase(false);
    policy.set_require_digits(false);
    policy.set_require_special(false);

    EXPECT_TRUE(policy.validate("ABCDEFGH"));
    EXPECT_FALSE(policy.validate("abcdefgh")); // no uppercase
}

/**
 * @brief Test password with only special characters
 * @details Verifies that a password composed entirely of punctuation
 *          satisfies the special character requirement
 */
TEST_F(PasswordPolicyTest, Validate_OnlySpecial)
{
    PasswordPolicy policy(4, 64);
    policy.set_require_uppercase(false);
    policy.set_require_lowercase(false);
    policy.set_require_digits(false);
    policy.set_require_special(true);

    EXPECT_TRUE(policy.validate("!@#$%^"));
    EXPECT_TRUE(policy.validate("...?!!"));
    EXPECT_FALSE(policy.validate("abcd")); // no special
}

/**
 * @brief Test the else-if chain detects multiple categories in one password
 * @details A single character like 'A' should be classified as uppercase,
 *          not as a special character (even though it's also std::ispunct=false).
 *          Verifies that all requirements are independently checked.
 */
TEST_F(PasswordPolicyTest, Validate_ElseIfChainMultiChar)
{
    PasswordPolicy policy(4, 64);
    policy.set_require_uppercase(true);
    policy.set_require_lowercase(true);
    policy.set_require_digits(true);
    policy.set_require_special(true);

    // Each char satisfies exactly one category in the else-if chain
    EXPECT_TRUE(policy.validate("Ab1!"));
    EXPECT_TRUE(policy.validate("A!1b"));
    EXPECT_TRUE(policy.validate("1bA!"));
}

/**
 * @brief Test validate with a password at exactly min_length boundary
 *        when min_length is a large value
 * @details Verifies allocation boundary for long passwords at limit
 */
TEST_F(PasswordPolicyTest, Validate_LargeMinLength)
{
    PasswordPolicy policy(1000, 2000);
    policy.set_require_uppercase(false);
    policy.set_require_lowercase(false);
    policy.set_require_digits(false);
    policy.set_require_special(false);

    const std::string exactMin(1000, 'x');
    const std::string oneShort(999, 'x');
    EXPECT_TRUE(policy.validate(exactMin));
    EXPECT_FALSE(policy.validate(oneShort));
}

/**
 * @brief Test setter for max_login_attempts and its getter
 * @details The max_login_attempts parameter is configured via constructor
 *          but not via individual setter
 */
TEST_F(PasswordPolicyTest, MaxLoginAttemptsBoundary)
{
    const PasswordPolicy policy(8, 64, true, true, true, true, 0);
    EXPECT_EQ(policy.max_login_attempts(), 0);

    const PasswordPolicy policyLarge(8, 64, true, true, true, true, SIZE_MAX);
    EXPECT_EQ(policyLarge.max_login_attempts(), SIZE_MAX);
}

/**
 * @brief Test password containing non-ASCII bytes (char > 127)
 * @details The implementation casts to unsigned char before passing to
 *          std::isupper etc. Verifies no undefined behavior for negative char values.
 */
TEST_F(PasswordPolicyTest, Validate_NonAsciiBytes)
{
    PasswordPolicy policy(2, 64);
    policy.set_require_uppercase(false);
    policy.set_require_lowercase(false);
    policy.set_require_digits(false);
    policy.set_require_special(false);

    // Bytes with high bit set (negative char values)
    EXPECT_TRUE(policy.validate("\x80\xff"));
    EXPECT_TRUE(policy.validate("\xa0\xc0"));
}
