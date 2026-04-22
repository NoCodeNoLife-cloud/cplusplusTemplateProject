#include "CaesarCipher.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <algorithm>
#include <limits>

namespace common::crypto::cipher {
    CaesarCipher::CaesarCipher(const int shift) : shift_(NormalizeShift(shift)) {
        DLOG(INFO) << fmt::format("CaesarCipher initialized with shift: {} (normalized from {})", shift_, shift);
    }

    auto CaesarCipher::Encrypt(const std::string_view plaintext) const -> std::string {
        DLOG(INFO) << fmt::format("CaesarCipher encrypt - shift: {}, input length: {}", shift_, plaintext.length());
        return Transform(plaintext, shift_);
    }

    auto CaesarCipher::Decrypt(const std::string_view ciphertext) const -> std::string {
        DLOG(INFO) << fmt::format("CaesarCipher decrypt - shift: {}, input length: {}", shift_, ciphertext.length());
        return Transform(ciphertext, -shift_);
    }

    auto CaesarCipher::Encrypt(const std::string_view text, const int shift) -> std::string {
        return CaesarCipher(shift).Encrypt(text);
    }

    auto CaesarCipher::Decrypt(const std::string_view text, const int shift) -> std::string {
        return CaesarCipher(shift).Decrypt(text);
    }

    auto CaesarCipher::IsValidInput(const std::string_view text) noexcept -> bool {
        return std::ranges::all_of(text, [](const unsigned char c) {
            return c <= 127;
        });
    }

    auto CaesarCipher::CreateRot13() -> CaesarCipher {
        return CaesarCipher(13);
    }

    auto CaesarCipher::SetShift(const int new_shift) -> void {
        const int old_shift = shift_;
        shift_ = NormalizeShift(new_shift);
        DLOG(INFO) << fmt::format("CaesarCipher shift changed - old: {}, new: {} (from {})", old_shift, shift_, new_shift);
    }

    auto CaesarCipher::Transform(const std::string_view text, const int effective_shift) -> std::string {
        if (!IsValidInput(text)) {
            DLOG(ERROR) << "CaesarCipher transform failed - input contains non-ASCII characters";
            throw std::invalid_argument("Input contains non-ASCII characters");
        }

        std::string result;
        result.reserve(text.length());

        for (const unsigned char c: text) {
            if (std::isalpha(c)) {
                result.push_back(ShiftChar(static_cast<char>(c), effective_shift));
            } else {
                result.push_back(static_cast<char>(c));
            }
        }

        DLOG(INFO) << fmt::format("CaesarCipher transform completed - output length: {}", result.length());
        return result;
    }

    auto CaesarCipher::ShiftChar(const char c, const int shift) noexcept -> char {
        const char base = std::isupper(c) ? 'A' : 'a';
        // Modular arithmetic ensures wrap-around handling
        int offset = (c - base + shift) % kAlphabetSize;
        if (offset < 0) offset += kAlphabetSize;
        return static_cast<char>(base + offset);
    }

    auto CaesarCipher::NormalizeShift(int shift) -> int {
        // Prevent abs(INT_MIN) undefined behavior
        if (shift == std::numeric_limits<int>::min()) {
            shift += kAlphabetSize;
        }

        int normalized = shift % kAlphabetSize;
        if (normalized < 0) {
            normalized += kAlphabetSize;
        }
        return normalized;
    }
} // namespace common::crypto::cipher
