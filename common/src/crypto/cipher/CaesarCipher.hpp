#pragma once
#include <string>
#include <string_view>
#include <stdexcept>

/**
 * @namespace common::crypto::cipher
 * @brief Cryptographic cipher implementations
 * @details Contains classical and modern cipher algorithms including
 *          substitution ciphers, stream ciphers, and block ciphers.
 */
namespace common::crypto::cipher {
    /**
     * @class CaesarCipher
     * @brief Implementation of the Caesar cipher substitution algorithm
     * @details A classic substitution cipher where each letter in the plaintext
     *          is 'shifted' a certain number of places down the alphabet. This
     *          implementation supports both positive (right shift) and negative
     *          (left shift) offsets, preserving case and non-alphabetic characters.
     *
     * @par Thread Safety:
     * This class is thread-safe for concurrent read operations (const methods).
     * Non-const operations (SetShift) require external synchronization.
     *
     * @par Usage Example:
     * @code
     * common::crypto::cipher::CaesarCipher cipher(3);
     * std::string encrypted = cipher.Encrypt("Hello, World!"); // "Khoor, Zruog!"
     * std::string decrypted = cipher.Decrypt(encrypted);         // "Hello, World!"
     *
     * // Static utility for one-shot operations
     * auto result = common::crypto::cipher::CaesarCipher::Encrypt("Attack", 5);
     * @endcode
     */
    class CaesarCipher final {
    public:
        /**
         * @brief Traditional Caesar shift amount (Julius Caesar used 3)
         */
        static constexpr int kDefaultShift = 3;

        /**
         * @brief Size of English alphabet
         */
        static constexpr int kAlphabetSize = 26;

        /**
         * @brief Constructs cipher with specified shift amount
         * @param[in] shift Positions to shift (positive=right, negative=left)
         * @throws std::invalid_argument If shift normalization fails
         * @post shift is normalized to [0, 25] range internally
         */
        explicit CaesarCipher(int shift = kDefaultShift);

        /** @brief Destructor */
        ~CaesarCipher() noexcept = default;

        /** @brief Copy constructor */
        constexpr CaesarCipher(const CaesarCipher &other) noexcept = default;

        /** @brief Copy assignment */
        constexpr auto operator=(const CaesarCipher &other) noexcept -> CaesarCipher & = default;

        /** @brief Move constructor */
        constexpr CaesarCipher(CaesarCipher &&other) noexcept = default;

        /** @brief Move assignment */
        constexpr auto operator=(CaesarCipher &&other) noexcept -> CaesarCipher & = default;

        /**
         * @brief Encrypts plaintext using configured shift
         * @param[in] plaintext ASCII string to encrypt
         * @return Ciphertext result
         * @throws std::invalid_argument If non-ASCII characters detected
         * @par Complexity: Time O(n), Space O(n)
         */
        [[nodiscard]] auto Encrypt(std::string_view plaintext) const -> std::string;

        /**
         * @brief Decrypts ciphertext using configured shift
         * @param[in] ciphertext ASCII string to decrypt
         * @return Plaintext result
         * @throws std::invalid_argument If non-ASCII characters detected
         * @par Complexity: Time O(n), Space O(n)
         */
        [[nodiscard]] auto Decrypt(std::string_view ciphertext) const -> std::string;

        /**
         * @brief Stateless encryption utility
         * @param[in] text Input text
         * @param[in] shift Shift amount for this operation only
         * @return Encrypted string
         * @throws std::invalid_argument If invalid input or shift
         */
        [[nodiscard]] static auto Encrypt(std::string_view text, int shift) -> std::string;

        /**
         * @brief Stateless decryption utility
         * @param[in] text Input text
         * @param[in] shift Original encryption shift
         * @return Decrypted string
         * @throws std::invalid_argument If invalid input or shift
         */
        [[nodiscard]] static auto Decrypt(std::string_view text, int shift) -> std::string;

        /**
         * @brief Input validation helper
         * @param[in] text String to validate
         * @return true if all chars are ASCII (0-127)
         */
        [[nodiscard]] static auto IsValidInput(std::string_view text) noexcept -> bool;

        /**
         * @brief Factory method for ROT13 variant
         * @return Configured instance with shift=13
         * @note ROT13 is self-inverse: Encrypt() == Decrypt()
         */
        [[nodiscard]] static auto CreateRot13() -> CaesarCipher;

        /**
         * @brief Current shift value accessor
         * @return Normalized shift in range [0, 25]
         */
        [[nodiscard]] auto GetShift() const noexcept -> int { return shift_; }

        /**
         * @brief Modifies shift amount
         * @param[in] new_shift New shift value
         * @throws std::invalid_argument If normalization fails
         */
        auto SetShift(int new_shift) -> void;

    private:
        /**
         * @brief Internal normalized shift value
         */
        int shift_;

        /**
         * @brief Core transformation engine
         * @param[in] text Input string
         * @param[in] effective_shift Applied shift (handles negative)
         * @return Transformed result
         */
        [[nodiscard]] static auto Transform(std::string_view text, int effective_shift) -> std::string;

        /**
         * @brief Character-level shift logic
         * @param[in] c Alphabet character to shift
         * @param[in] shift Amount to shift
         * @return Shifted character preserving case
         * @pre std::isalpha(c) == true
         */
        [[nodiscard]] static auto ShiftChar(char c, int shift) noexcept -> char;

        /**
         * @brief Normalizes any integer to valid shift range
         * @param[in] shift Raw shift value
         * @return Value in [0, 25]
         * @throws std::invalid_argument On arithmetic overflow
         */
        [[nodiscard]] static auto NormalizeShift(int shift) -> int;
    };
} // namespace common::crypto::cipher
