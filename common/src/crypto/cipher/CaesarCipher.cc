/**
 * @file CaesarCipher.cc
 * @brief CaesarCipher implementation — shift encryption/decryption
 * @details Implements the Caesar cipher: shift each alphabetic character by
 *          a fixed offset, preserving case and non-alphabetic characters.
 */

#include "CaesarCipher.hpp"

#include <algorithm>
#include <limits>
#include <ranges>
#include <glog/logging.h>

namespace common::crypto::cipher
{
    CaesarCipher::CaesarCipher(const int shift) : shift_(NormalizeShift(shift))
    {
    }

    std::string CaesarCipher::Encrypt(const std::string_view plaintext) const
    {
        return Transform(plaintext, shift_);
    }

    std::string CaesarCipher::Decrypt(const std::string_view ciphertext) const
    {
        return Transform(ciphertext, -shift_);
    }

    std::string CaesarCipher::Encrypt(const std::string_view text, const int shift)
    {
        return CaesarCipher(shift).Encrypt(text);
    }

    std::string CaesarCipher::Decrypt(const std::string_view text, const int shift)
    {
        return CaesarCipher(shift).Decrypt(text);
    }

    bool CaesarCipher::IsValidInput(const std::string_view text)
    {
        return std::ranges::all_of(text, [](const unsigned char c)
        {
            return c <= 127;
        });
    }

    CaesarCipher CaesarCipher::CreateRot13()
    {
        return CaesarCipher(13);
    }

    void CaesarCipher::SetShift(const int new_shift)
    {
        shift_ = NormalizeShift(new_shift);
    }

    std::string CaesarCipher::Transform(const std::string_view text, const int effective_shift)
    {
        if (!IsValidInput(text))
        {
            DLOG(WARNING) << "Caesar cipher input contains non-ASCII characters";
            throw std::invalid_argument("Input contains non-ASCII characters");
        }

        auto transformed = text | std::ranges::views::transform([effective_shift](const unsigned char c)
        {
            return std::isalpha(c) ? ShiftChar(static_cast<char>(c), effective_shift) : static_cast<char>(c);
        });

        return {transformed.begin(), transformed.end()};
    }

    char CaesarCipher::ShiftChar(const char c, const int shift)
    {
        const char base = std::isupper(c) ? 'A' : 'a';
        // Modular arithmetic ensures wrap-around handling
        int offset = (c - base + shift) % kAlphabetSize;
        if (offset < 0) offset += kAlphabetSize;
        return static_cast<char>(base + offset);
    }

    int CaesarCipher::NormalizeShift(int shift)
    {
        // Prevent abs(INT_MIN) undefined behavior
        if (shift == std::numeric_limits<int>::min())
        {
            shift += kAlphabetSize;
        }

        int normalized = shift % kAlphabetSize;
        if (normalized < 0)
        {
            normalized += kAlphabetSize;
        }
        return normalized;
    }
}
