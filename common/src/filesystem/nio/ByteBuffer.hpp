#pragma once
#include <span>
#include <vector>

#include "src/interface/IBuffer.hpp"

namespace common::filesystem {
    /// @brief A byte buffer implementation that manages a fixed-size buffer of bytes.
    /// This class provides methods for putting and getting bytes, supporting both
    /// single-byte operations and bulk operations with vectors of bytes.
    class ByteBuffer final : public interfaces::IBuffer {
    public:
        explicit ByteBuffer(size_t capacity);

        /// @brief Get the capacity of the buffer
        /// @return The capacity of the buffer
        [[nodiscard]] auto capacity() const noexcept -> size_t override;

        /// @brief Get the current position of the buffer
        /// @return The current position of the buffer
        [[nodiscard]] auto position() const noexcept -> size_t override;

        /// @brief Set the position of the buffer
        /// @param newPosition The new position to set
        /// @throws std::out_of_range If newPosition exceeds the current limit
        auto position(size_t newPosition) -> void override;

        /// @brief Get the limit of the buffer
        /// @return The limit of the buffer
        [[nodiscard]] auto limit() const noexcept -> size_t override;

        /// @brief Set the limit of the buffer
        /// @param newLimit The new limit to set
        /// @throws std::out_of_range If newLimit exceeds capacity
        auto limit(size_t newLimit) -> void override;

        /// @brief Clear the buffer by resetting position and limit
        auto clear() noexcept -> void override;

        /// @brief Flip the buffer by setting limit to current position and resetting position to zero
        auto flip() noexcept -> void override;

        /// @brief Rewind the buffer by resetting position to zero
        auto rewind() noexcept -> void override;

        /// @brief Get the number of remaining elements in the buffer
        /// @return The number of remaining elements
        [[nodiscard]] auto remaining() const noexcept -> size_t override;

        /// @brief Check if there are remaining elements in the buffer
        /// @return True if there are remaining elements, false otherwise
        [[nodiscard]] auto hasRemaining() const noexcept -> bool override;

        /// @brief Puts a single byte into the buffer
        /// @param value The byte value to put into the buffer
        /// @throws std::overflow_error If buffer has no remaining space
        auto put(std::byte value) -> void;

        /// @brief Puts bytes from a vector into the buffer
        /// @param src The source vector of bytes to put into the buffer
        /// @throws std::overflow_error If buffer has insufficient space
        auto put(const std::vector<std::byte> &src) -> void;

        /// @brief Gets a single byte from the buffer
        /// @return The byte retrieved from the buffer
        /// @throws std::underflow_error If buffer has no remaining data
        [[nodiscard]] auto get() -> std::byte;

        /// @brief Gets a specified number of bytes from the buffer
        /// @param length The number of bytes to retrieve
        /// @return A vector containing the retrieved bytes
        /// @throws std::underflow_error If buffer has insufficient data
        [[nodiscard]] auto get(size_t length) -> std::vector<std::byte>;

    private:
        std::vector<std::byte> buffer_;
    };
}
