#pragma once
#include <algorithm>
#include <string>

#include "src/interface/IBuffer.hpp"

namespace common::filesystem {
    /// @brief A character buffer that implements the IBuffer interface.
    /// @details This class provides functionality for managing a buffer of characters,
    ///          including operations such as putting and getting characters, compacting
    ///          the buffer, and retrieving remaining unprocessed characters.
    class CharBuffer final : public interfaces::IBuffer {
    public:
        /// @brief Constructs a CharBuffer with the specified capacity.
        /// @param cap The capacity of the buffer.
        explicit CharBuffer(size_t cap);

        /// @brief Clears the buffer by setting position to 0 and limit to capacity.
        /// @return void
        auto clear() noexcept -> void override;

        /// @brief Flips the buffer by setting limit to current position and position to 0.
        /// @return void
        auto flip() noexcept -> void override;

        /// @brief Resets the position to 0 while keeping the limit unchanged.
        /// @return void
        auto rewind() noexcept -> void override;

        /// @brief Compacts the buffer by removing processed characters.
        /// @return void
        auto compact() -> void;

        /// @brief Puts a single character into the buffer.
        /// @param c The character to put into the buffer.
        /// @return void
        auto put(char c) -> void;

        /// @brief Puts a string into the buffer.
        /// @param src The string to put into the buffer.
        /// @return void
        auto put(const std::string &src) -> void;

        /// @brief Gets a single character from the buffer.
        /// @return The character retrieved from the buffer.
        [[nodiscard]] auto get() -> char;

        /// @brief Gets the remaining unprocessed characters as a string.
        /// @return The remaining characters in the buffer as a string.
        [[nodiscard]] auto getRemaining() const -> std::string;

        /// @brief Returns the current position in the buffer.
        /// @return The current position.
        [[nodiscard]] auto position() const noexcept -> size_t override;

        /// @brief Sets the position in the buffer.
        /// @param newPosition The new position to set.
        /// @return void
        auto position(size_t newPosition) -> void override;

        /// @brief Returns the limit of the buffer.
        /// @return The current limit.
        [[nodiscard]] auto limit() const noexcept -> size_t override;

        /// @brief Sets the limit of the buffer.
        /// @param newLimit The new limit to set.
        /// @return void
        auto limit(size_t newLimit) -> void override;

        /// @brief Returns the capacity of the buffer.
        /// @return The buffer capacity.
        [[nodiscard]] auto capacity() const noexcept -> size_t override;

        /// @brief Checks if there are remaining elements in the buffer.
        /// @return true if there are remaining elements, false otherwise.
        [[nodiscard]] auto hasRemaining() const noexcept -> bool override;

        /// @brief Returns the number of remaining elements in the buffer.
        /// @return The number of remaining elements.
        [[nodiscard]] auto remaining() const noexcept -> size_t override;

    private:
        std::string buffer_{};
    };
}
