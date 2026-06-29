/**
 * @file IBuffer.hpp
 * @brief Interface for NIO-style buffers with position/limit/capacity
 * @description Defines the contract for buffer types (ByteBuffer, CharBuffer,
 *          etc.): position/limit/capacity management, flip/clear/rewind/compact
 *          state transitions, and remaining element queries.  Analogous to
 *          java.nio.Buffer.
 */

#pragma once
#include <cstddef>
#include <stdexcept>

namespace cppforge::interface::io
{
    /// @brief Interface for a buffer that manages a sequence of elements
    /// This interface provides methods for managing a buffer's position, limit, and capacity.
    /// The buffer follows the standard buffer semantics where:
    /// - Capacity is the maximum number of elements the buffer can hold
    /// - Limit is the index of the first element that should not be read/written
    /// - Position is the index of the next element to be read/written
    class IBuffer
    {
    public:
        virtual ~IBuffer() = default;

        /// @brief Get the capacity of the buffer
        /// @return The capacity of the buffer
        [[nodiscard]] virtual size_t capacity() const  = 0;

        /// @brief Get the current position of the buffer
        /// @return The current position of the buffer
        [[nodiscard]] virtual size_t position() const  = 0;

        /// @brief Set the position of the buffer
        /// @param newPosition The new position to set
        /// @throws std::out_of_range If newPosition exceeds the current limit
        virtual void position(size_t newPosition) = 0;

        /// @brief Get the limit of the buffer
        /// @return The limit of the buffer
        [[nodiscard]] virtual size_t limit() const  = 0;

        /// @brief Set the limit of the buffer
        /// @param newLimit The new limit to set
        /// @throws std::out_of_range If newLimit exceeds capacity
        virtual void limit(size_t newLimit) = 0;

        /// @brief Clear the buffer by resetting position and limit
        virtual void clear() = 0;

        /// @brief Flip the buffer by setting limit to current position and resetting position to zero
        virtual void flip() = 0;

        /// @brief Rewind the buffer by resetting position to zero
        virtual void rewind() = 0;

        /// @brief Get the number of remaining elements in the buffer
        /// @return The number of remaining elements
        [[nodiscard]] virtual size_t remaining() const  = 0;

        /// @brief Check if there are remaining elements in the buffer
        /// @return True if there are remaining elements, false otherwise
        [[nodiscard]] virtual bool hasRemaining() const  = 0;

    protected:
        size_t position_{0};
        size_t limit_{0};
        size_t capacity_{0};
    };
}
