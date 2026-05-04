#pragma once
#include <boost/uuid/uuid.hpp>
#include <string>

namespace common::gen {
    /// @brief A utility class for generating UUIDs.
    /// This class provides functionality to generate random UUID strings.
    class UuidGenerator {
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
        [[nodiscard]] static auto GenerateRandomUuid() noexcept -> std::string;
    };
}
