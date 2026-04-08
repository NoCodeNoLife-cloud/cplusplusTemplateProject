#include "CaesarCipher.hpp"

#include <algorithm>
#include <limits>

namespace common {
    CaesarCipher::CaesarCipher(int shift) : shift_(NormalizeShift(shift)) {
    }

    std::string CaesarCipher::Encrypt(std::string_view plaintext) const {
        return Transform(plaintext, shift_);
    }

    std::string CaesarCipher::Decrypt(std::string_view ciphertext) const {
        return Transform(ciphertext, -shift_);
    }

    std::string CaesarCipher::Encrypt(std::string_view text, int shift) {
        return CaesarCipher(shift).Encrypt(text);
    }

    std::string CaesarCipher::Decrypt(std::string_view text, int shift) {
        return CaesarCipher(shift).Decrypt(text);
    }

    bool CaesarCipher::IsValidInput(std::string_view text) noexcept {
        return std::ranges::all_of(text, [](unsigned char c) {
            return c <= 127;
        });
    }

    CaesarCipher CaesarCipher::CreateRot13() {
        return CaesarCipher(13);
    }

    void CaesarCipher::SetShift(int new_shift) {
        shift_ = NormalizeShift(new_shift);
    }

    std::string CaesarCipher::Transform(std::string_view text, int effective_shift) {
        if (!IsValidInput(text)) {
            throw std::invalid_argument("Input contains non-ASCII characters");
        }

        std::string result;
        result.reserve(text.length());

        for (unsigned char c: text) {
            if (std::isalpha(c)) {
                result.push_back(ShiftChar(static_cast<char>(c), effective_shift));
            } else {
                result.push_back(static_cast<char>(c));
            }
        }

        return result;
    }

    char CaesarCipher::ShiftChar(char c, int shift) noexcept {
        const char base = std::isupper(c) ? 'A' : 'a';
        // Modular arithmetic ensures wrap-around handling
        int offset = (c - base + shift) % kAlphabetSize;
        if (offset < 0) offset += kAlphabetSize;
        return static_cast<char>(base + offset);
    }

    int CaesarCipher::NormalizeShift(int shift) {
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
}
