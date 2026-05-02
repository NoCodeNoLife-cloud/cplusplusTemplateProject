#pragma once
#include <stdexcept>
#include <vector>

#include "src/interface/IBuffer.hpp"

namespace common::filesystem {
    /// @brief A buffer that stores 64-bit signed integers
    class LongBuffer final : public interfaces::IBuffer {
    public:
        /// @brief Construct a LongBuffer with the specified capacity
        /// @param capacity The initial capacity of the buffer
        explicit LongBuffer(std::size_t capacity);

        /// @brief Get the current value from the buffer and advance the position
        /// @return The current value
        /// @throws std::out_of_range If no remaining elements to get
        [[nodiscard]] auto get() -> int64_t;

        /// @brief Put a value into the buffer and advance the position
        /// @param value The value to put
        /// @throws std::out_of_range If no remaining space to put
        auto put(int64_t value) -> void;

        /// @brief Check if there are remaining elements in the buffer
        /// @return True if there are remaining elements, false otherwise
        [[nodiscard]] auto hasRemaining() const noexcept -> bool override;

        /// @brief Get the number of remaining elements in the buffer
        /// @return Number of remaining elements
        [[nodiscard]] auto remaining() const noexcept -> std::size_t override;

        /// @brief Get the current position in the buffer
        /// @return The current position
        [[nodiscard]] auto position() const noexcept -> std::size_t override;

        /// @brief Set the position in the buffer
        /// @param newPosition The new position to set
        /// @throws std::out_of_range If the new position is out of range
        auto position(std::size_t newPosition) -> void override;

        /// @brief Get the limit of the buffer
        /// @return The current limit
        [[nodiscard]] auto limit() const noexcept -> std::size_t override;

        /// @brief Set the limit of the buffer
        /// @param newLimit The new limit to set
        /// @throws std::out_of_range If the new limit is out of range
        auto limit(std::size_t newLimit) -> void override;

        /// @brief Get the capacity of the buffer
        /// @return The capacity
        [[nodiscard]] auto capacity() const noexcept -> std::size_t override;

        /// @brief Reset the buffer position to zero and set limit to capacity
        auto clear() noexcept -> void override;

        /// @brief Flip the buffer (limit = position, position = 0)
        auto flip() noexcept -> void override;

        /// @brief Reset the buffer position to zero
        auto rewind() noexcept -> void override;

    private:
        std::vector<int64_t> buffer_{};
        std::size_t capacity_{};
        std::size_t limit_{};
        std::size_t position_{};
    };
}
