#pragma once
#include <stdexcept>

namespace common::interfaces {
    /// @brief Interface for a buffer that manages a sequence of elements
    /// This interface provides methods for managing a buffer's position, limit, and capacity.
    /// The buffer follows the standard buffer semantics where:
    /// - Capacity is the maximum number of elements the buffer can hold
    /// - Limit is the index of the first element that should not be read/written
    /// - Position is the index of the next element to be read/written
    class IBuffer {
    public:
        virtual ~IBuffer() = default;

        /// @brief Get the capacity of the buffer
        /// @return The capacity of the buffer
        [[nodiscard]] virtual auto capacity() const noexcept -> size_t = 0;

        /// @brief Get the current position of the buffer
        /// @return The current position of the buffer
        [[nodiscard]] virtual auto position() const noexcept -> size_t = 0;

        /// @brief Set the position of the buffer
        /// @param newPosition The new position to set
        /// @throws std::out_of_range If newPosition exceeds the current limit
        virtual auto position(size_t newPosition) -> void = 0;

        /// @brief Get the limit of the buffer
        /// @return The limit of the buffer
        [[nodiscard]] virtual auto limit() const noexcept -> size_t = 0;

        /// @brief Set the limit of the buffer
        /// @param newLimit The new limit to set
        /// @throws std::out_of_range If newLimit exceeds capacity
        virtual auto limit(size_t newLimit) -> void = 0;

        /// @brief Clear the buffer by resetting position and limit
        virtual auto clear() -> void = 0;

        /// @brief Flip the buffer by setting limit to current position and resetting position to zero
        virtual auto flip() -> void = 0;

        /// @brief Rewind the buffer by resetting position to zero
        virtual auto rewind() -> void = 0;

        /// @brief Get the number of remaining elements in the buffer
        /// @return The number of remaining elements
        [[nodiscard]] virtual auto remaining() const noexcept -> size_t = 0;

        /// @brief Check if there are remaining elements in the buffer
        /// @return True if there are remaining elements, false otherwise
        [[nodiscard]] virtual auto hasRemaining() const noexcept -> bool = 0;

    protected:
        size_t position_{0};
        size_t limit_{0};
        size_t capacity_{0};
    };
}
