/**
 * @file CaesarCipher.cc
 * @brief CaesarCipher class implementation
 * @details This file contains the implementation of the CaesarCipher class methods for Cryptographic utilities and toolkit.
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

    auto CaesarCipher::Encrypt(const std::string_view plaintext) const -> std::string
    {
        return Transform(plaintext, shift_);
    }

    auto CaesarCipher::Decrypt(const std::string_view ciphertext) const -> std::string
    {
        return Transform(ciphertext, -shift_);
    }

    auto CaesarCipher::Encrypt(const std::string_view text, const int shift) -> std::string
    {
        return CaesarCipher(shift).Encrypt(text);
    }

    auto CaesarCipher::Decrypt(const std::string_view text, const int shift) -> std::string
    {
        return CaesarCipher(shift).Decrypt(text);
    }

    auto CaesarCipher::IsValidInput(const std::string_view text) noexcept -> bool
    {
        return std::ranges::all_of(text, [](const unsigned char c)
        {
            return c <= 127;
        });
    }

    auto CaesarCipher::CreateRot13() -> CaesarCipher
    {
        return CaesarCipher(13);
    }

    auto CaesarCipher::SetShift(const int new_shift) -> void
    {
        shift_ = NormalizeShift(new_shift);
    }

    auto CaesarCipher::Transform(const std::string_view text, const int effective_shift) -> std::string
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

    auto CaesarCipher::ShiftChar(const char c, const int shift) noexcept -> char
    {
        const char base = std::isupper(c) ? 'A' : 'a';
        // Modular arithmetic ensures wrap-around handling
        int offset = (c - base + shift) % kAlphabetSize;
        if (offset < 0) offset += kAlphabetSize;
        return static_cast<char>(base + offset);
    }

    auto CaesarCipher::NormalizeShift(int shift) -> int
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
} // namespace common::crypto::cipher
