/**
 * @file SubstitutionCipherTest.cc
 * @brief Unit tests for SubstitutionCipher (shift cipher).
 * @details Contains 14 test cases covering default shift, custom shift,
 *          negative shift, large-shift normalization, round-trip encryption,
 *          mapping constructor, partial mapping, non-alphabetic character
 *          preservation, case preservation, empty string handling, get-mapping,
 *          and move semantics.
 */

#include <gtest/gtest.h>

#include <cppforge/crypto/cipher/SubstitutionCipher.hpp>

using namespace cppforge::crypto::cipher;

/// @brief Test fixture for SubstitutionCipher tests.
class SubstitutionCipherTest : public testing::Test
{
protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

/** @brief Default shift (3) produces correct encryption.
 *  @details Verifies that the default-constructed SubstitutionCipher applies a shift of 3 to both lowercase letters and mixed-case text, preserving non-alphabetic characters. */
TEST_F(SubstitutionCipherTest, DefaultShift)
{
    const SubstitutionCipher cipher;
    EXPECT_EQ(cipher.Encrypt("abc"), "def");
    EXPECT_EQ(cipher.Encrypt("xyz"), "abc");
    EXPECT_EQ(cipher.Encrypt("Hello, World!"), "Khoor, Zruog!");
}

/** @brief Default shift decryption reverses encryption.
 *  @details Verifies that Decrypt with default shift of 3 correctly reverses the Encrypt operation, including wrap-around from 'a' to 'x'/'z' to 'w'. */
TEST_F(SubstitutionCipherTest, Decrypt_DefaultShift)
{
    const SubstitutionCipher cipher;
    EXPECT_EQ(cipher.Decrypt("def"), "abc");
    EXPECT_EQ(cipher.Decrypt("abc"), "xyz");
    EXPECT_EQ(cipher.Decrypt("Khoor, Zruog!"), "Hello, World!");
}

/** @brief Round-trip encryption/decryption with custom shift.
 *  @details Encrypts and decrypts a full sentence with shift 7, asserting that the decrypted output matches the original plaintext exactly. */
TEST_F(SubstitutionCipherTest, RoundTrip)
{
    const std::string plaintext = "The Quick Brown Fox Jumps Over The Lazy Dog!";
    const SubstitutionCipher cipher(7);

    const auto ciphertext = cipher.Encrypt(plaintext);
    const auto decrypted = cipher.Decrypt(ciphertext);

    EXPECT_EQ(decrypted, plaintext);
}

/** @brief Custom shift of 13 (ROT13).
 *  @details Verifies that a shift of 13 produces the expected ROT13 ciphertext and that decryption correctly recovers the plaintext. */
TEST_F(SubstitutionCipherTest, CustomShift)
{
    const SubstitutionCipher cipher(13);
    EXPECT_EQ(cipher.Encrypt("hello"), "uryyb");
    EXPECT_EQ(cipher.Decrypt("uryyb"), "hello");
}

/** @brief Negative shift values are supported.
 *  @details Verifies that a shift of -3 correctly shifts letters backward and that encryption and decryption are symmetric opposites. */
TEST_F(SubstitutionCipherTest, NegativeShift)
{
    const SubstitutionCipher cipher(-3);
    EXPECT_EQ(cipher.Encrypt("def"), "abc");
    EXPECT_EQ(cipher.Decrypt("abc"), "def");
}

/** @brief Large shift values wrap around modulo 26.
 *  @details Verifies that a shift of 29 (â‰?3 mod 26) produces identical output to a shift of 3, confirming proper modulo normalization. */
TEST_F(SubstitutionCipherTest, LargeShiftNormalization)
{
    const SubstitutionCipher cipher1(29);
    const SubstitutionCipher cipher2(3);

    EXPECT_EQ(cipher1.Encrypt("test"), cipher2.Encrypt("test"));
}

/** @brief Deterministic encryption with same seed.
 *  @details Verifies that two cipher instances constructed with the same seed produce identical encryption output, and that decryption correctly recovers the original input. */
TEST_F(SubstitutionCipherTest, RandomSeed)
{
    const SubstitutionCipher cipher1(42u);
    const SubstitutionCipher cipher2(42u);

    const std::string input = "Hello World";
    EXPECT_EQ(cipher1.Encrypt(input), cipher2.Encrypt(input));
    EXPECT_EQ(cipher1.Decrypt(cipher1.Encrypt(input)), input);
}

/** @brief Custom character mapping via constructor.
 *  @details Verifies that a SubstitutionCipher constructed from an explicit char-to-char mapping encrypts and decrypts correctly according to the provided mapping. */
TEST_F(SubstitutionCipherTest, MappingConstructor)
{
    const std::unordered_map<char, char> mapping = {
        {'A', 'Q'}, {'B', 'W'}, {'C', 'E'},
        {'Q', 'A'}, {'W', 'B'}, {'E', 'C'},
        {'a', 'q'}, {'b', 'w'}, {'c', 'e'},
        {'q', 'a'}, {'w', 'b'}, {'e', 'c'}
    };
    const SubstitutionCipher cipher(mapping);

    EXPECT_EQ(cipher.Encrypt("abc"), "qwe");
    EXPECT_EQ(cipher.Decrypt("qwe"), "abc");
}

/** @brief Partial mapping leaves unmapped characters unchanged.
 *  @details Verifies that a mapping covering only 'A'/'Z' and 'a'/'z' leaves letters like 'B'/'b' unchanged during encryption. */
TEST_F(SubstitutionCipherTest, PartialMapping)
{
    const std::unordered_map<char, char> mapping = {
        {'A', 'Z'}, {'Z', 'A'},
        {'a', 'z'}, {'z', 'a'}
    };
    const SubstitutionCipher cipher(mapping);

    EXPECT_EQ(cipher.Encrypt("A"), "Z");
    EXPECT_EQ(cipher.Encrypt("a"), "z");
    EXPECT_EQ(cipher.Encrypt("B"), "B");
}

/** @brief Non-alphabetic characters pass through unchanged.
 *  @details Verifies that digits, punctuation, and symbols are preserved in their original positions during encryption. */
TEST_F(SubstitutionCipherTest, NonAlphabeticCharacters)
{
    const SubstitutionCipher cipher(3);
    const std::string input = "Hello, World! 123 @#$";
    const auto encrypted = cipher.Encrypt(input);

    EXPECT_EQ(encrypted[5], ',');
    EXPECT_EQ(encrypted.substr(14, 3), "123");
    EXPECT_EQ(encrypted.substr(18, 3), "@#$");
}

/** @brief Encryption preserves character case.
 *  @details Verifies that the shifted character retains its original case â€?lowercase maps to lowercase, uppercase to uppercase. */
TEST_F(SubstitutionCipherTest, CasePreservation)
{
    const SubstitutionCipher cipher(3);
    const auto encrypted = cipher.Encrypt("aBcDeF");

    EXPECT_EQ(encrypted, "dEfGhI");
    EXPECT_TRUE(std::islower(static_cast<unsigned char>(encrypted[0])));
    EXPECT_TRUE(std::isupper(static_cast<unsigned char>(encrypted[1])));
}

/** @brief Empty string encrypts/decrypts to empty string.
 *  @details Verifies that both Encrypt and Decrypt handle empty input gracefully, returning an empty string. */
TEST_F(SubstitutionCipherTest, EmptyString)
{
    const SubstitutionCipher cipher(5);
    EXPECT_EQ(cipher.Encrypt(""), "");
    EXPECT_EQ(cipher.Decrypt(""), "");
}

/** @brief GetMapping returns the internal shift mapping array.
 *  @details Verifies that GetMapping returns a 26-element array where the first element maps 'a' to the shifted letter ('b' for shift 1). */
TEST_F(SubstitutionCipherTest, GetMapping)
{
    const SubstitutionCipher cipher(1);
    const auto& map = cipher.GetMapping();

    EXPECT_EQ(map.size(), 26u);
    EXPECT_EQ(map[0], 'B');
}

/** @brief SubstitutionCipher is move-constructible.
 *  @details Verifies that a SubstitutionCipher can be move-constructed and the moved-to instance produces correct encryption results. */
TEST_F(SubstitutionCipherTest, MoveConstructible)
{
    SubstitutionCipher cipher1(3);
    const SubstitutionCipher cipher2(std::move(cipher1));

    EXPECT_EQ(cipher2.Encrypt("abc"), "def");
}
