/**
 * @file UuidGenerator.hpp
 * @brief UUID v4 (random) generator using Boost.UUID
 * @description Generates universally unique identifiers (UUID v4, random-based)
 *          using the Boost.UUID library.  Produces UUIDs in standard string
 *          format (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx).  Thread-safe if the
 *          underlying random number generator is properly seeded.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Usage Example
 * @code
 * UuidGenerator gen;
 * std::string id = gen.generate(); // e.g. "f47ac10b-58cc-4372-a567-0e02b2c3d479"
 * @endcode
 */

#pragma once
#include <string>
#include <boost/uuid/uuid.hpp>

namespace common::gen
{
    /// @brief A utility class for generating UUIDs.
    /// This class provides functionality to generate random UUID strings.
    class UuidGenerator
    {
    public:
        /// @brief UUID string total length (32 hex digits + 4 hyphens)
        static constexpr size_t UUID_STRING_LENGTH = 36;

        /// @brief UUID hyphen positions (0-indexed)
        static constexpr size_t HYPHEN_POSITION_1 = 8;
        static constexpr size_t HYPHEN_POSITION_2 = 13;
        static constexpr size_t HYPHEN_POSITION_3 = 18;
        static constexpr size_t HYPHEN_POSITION_4 = 23;

        /// @brief Number of hyphens in UUID format
        static constexpr size_t HYPHEN_COUNT = 4;

        /// @brief Number of hexadecimal characters in UUID (excluding hyphens)
        static constexpr size_t HEX_DIGIT_COUNT = 32;

        UuidGenerator() = delete;

        /// @brief Generates a random UUID string.
        /// @return A string representation of the generated UUID.
        [[nodiscard]] static std::string GenerateRandomUuid() noexcept;
    };
}