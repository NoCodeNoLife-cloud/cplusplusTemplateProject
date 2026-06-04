#include <gtest/gtest.h>

#include "crypto/cipher/SubstitutionCipher.hpp"

using namespace common::crypto::cipher;

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

TEST_F(SubstitutionCipherTest, DefaultShift)
{
    const SubstitutionCipher cipher;
    EXPECT_EQ(cipher.Encrypt("abc"), "def");
    EXPECT_EQ(cipher.Encrypt("xyz"), "abc");
    EXPECT_EQ(cipher.Encrypt("Hello, World!"), "Khoor, Zruog!");
}

TEST_F(SubstitutionCipherTest, Decrypt_DefaultShift)
{
    const SubstitutionCipher cipher;
    EXPECT_EQ(cipher.Decrypt("def"), "abc");
    EXPECT_EQ(cipher.Decrypt("abc"), "xyz");
    EXPECT_EQ(cipher.Decrypt("Khoor, Zruog!"), "Hello, World!");
}

TEST_F(SubstitutionCipherTest, RoundTrip)
{
    const std::string plaintext = "The Quick Brown Fox Jumps Over The Lazy Dog!";
    const SubstitutionCipher cipher(7);

    const auto ciphertext = cipher.Encrypt(plaintext);
    const auto decrypted = cipher.Decrypt(ciphertext);

    EXPECT_EQ(decrypted, plaintext);
}

TEST_F(SubstitutionCipherTest, CustomShift)
{
    const SubstitutionCipher cipher(13);
    EXPECT_EQ(cipher.Encrypt("hello"), "uryyb");
    EXPECT_EQ(cipher.Decrypt("uryyb"), "hello");
}

TEST_F(SubstitutionCipherTest, NegativeShift)
{
    const SubstitutionCipher cipher(-3);
    EXPECT_EQ(cipher.Encrypt("def"), "abc");
    EXPECT_EQ(cipher.Decrypt("abc"), "def");
}

TEST_F(SubstitutionCipherTest, LargeShiftNormalization)
{
    const SubstitutionCipher cipher1(29);
    const SubstitutionCipher cipher2(3);

    EXPECT_EQ(cipher1.Encrypt("test"), cipher2.Encrypt("test"));
}

TEST_F(SubstitutionCipherTest, RandomSeed)
{
    const SubstitutionCipher cipher1(42u);
    const SubstitutionCipher cipher2(42u);

    const std::string input = "Hello World";
    EXPECT_EQ(cipher1.Encrypt(input), cipher2.Encrypt(input));
    EXPECT_EQ(cipher1.Decrypt(cipher1.Encrypt(input)), input);
}

TEST_F(SubstitutionCipherTest, MappingConstructor)
{
    const std::unordered_map<char, char> mapping = {
        {'A', 'Q'}, {'B', 'W'}, {'C', 'E'},
        {'a', 'q'}, {'b', 'w'}, {'c', 'e'}
    };
    const SubstitutionCipher cipher(mapping);

    EXPECT_EQ(cipher.Encrypt("abc"), "qwe");
    EXPECT_EQ(cipher.Decrypt("qwe"), "abc");
}

TEST_F(SubstitutionCipherTest, PartialMapping)
{
    const std::unordered_map<char, char> mapping = {
        {'A', 'Z'}, {'a', 'z'}
    };
    const SubstitutionCipher cipher(mapping);

    EXPECT_EQ(cipher.Encrypt("A"), "Z");
    EXPECT_EQ(cipher.Encrypt("a"), "z");
    EXPECT_EQ(cipher.Encrypt("B"), "B");
}

TEST_F(SubstitutionCipherTest, NonAlphabeticCharacters)
{
    const SubstitutionCipher cipher(3);
    const std::string input = "Hello, World! 123 @#$";
    const auto encrypted = cipher.Encrypt(input);

    EXPECT_EQ(encrypted[5], ',');
    EXPECT_EQ(encrypted.substr(14, 3), "123");
    EXPECT_EQ(encrypted.substr(18, 3), "@#$");
}

TEST_F(SubstitutionCipherTest, CasePreservation)
{
    const SubstitutionCipher cipher(3);
    const auto encrypted = cipher.Encrypt("aBcDeF");

    EXPECT_EQ(encrypted, "dEfGhI");
    EXPECT_TRUE(std::islower(static_cast<unsigned char>(encrypted[0])));
    EXPECT_TRUE(std::isupper(static_cast<unsigned char>(encrypted[1])));
}

TEST_F(SubstitutionCipherTest, EmptyString)
{
    const SubstitutionCipher cipher(5);
    EXPECT_EQ(cipher.Encrypt(""), "");
    EXPECT_EQ(cipher.Decrypt(""), "");
}

TEST_F(SubstitutionCipherTest, GetMapping)
{
    const SubstitutionCipher cipher(1);
    const auto& map = cipher.GetMapping();

    EXPECT_EQ(map.size(), 26u);
    EXPECT_EQ(map[0], 'B');
}

TEST_F(SubstitutionCipherTest, MoveConstructible)
{
    SubstitutionCipher cipher1(3);
    const SubstitutionCipher cipher2(std::move(cipher1));

    EXPECT_EQ(cipher2.Encrypt("abc"), "def");
}
