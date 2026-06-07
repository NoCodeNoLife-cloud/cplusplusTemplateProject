/**
 * @file LongBuffer.hpp
 * @brief LongBuffer class declaration
 * @details This header defines the LongBuffer class that provides functionality for Common library utilities.
 */

#pragma once
#include <stdexcept>
#include <vector>

#include "interface/io/IBuffer.hpp"

namespace common::filesystem::nio
{
    /// @brief A buffer that stores 64-bit signed integers
    class LongBuffer final : public interface::io::IBuffer
    {
    public:
        /// @brief Construct a LongBuffer with the specified capacity
        /// @param capacity The initial capacity of the buffer
        explicit LongBuffer(std::size_t capacity);

        /// @brief Get the current value from the buffer and advance the position
        /// @return The current value
        /// @throws std::underflow_error If no remaining elements to get
        [[nodiscard]] int64_t get();

        /// @brief Put a value into the buffer and advance the position
        /// @param value The value to put
        /// @throws std::overflow_error If no remaining space to put
        void put(int64_t value);

        /// @brief Compacts the buffer by removing processed elements
        void compact();

        /// @brief Gets the remaining unprocessed elements as a vector
        /// @return The remaining elements in the buffer as a vector
        [[nodiscard]] std::vector<int64_t> getRemaining() const;

        /// @brief Check if there are remaining elements in the buffer
        /// @return True if there are remaining elements, false otherwise
        [[nodiscard]] bool hasRemaining() const  override;

        /// @brief Get the number of remaining elements in the buffer
        /// @return Number of remaining elements
        [[nodiscard]] std::size_t remaining() const  override;

        /// @brief Get the current position in the buffer
        /// @return The current position
        [[nodiscard]] std::size_t position() const  override;

        /// @brief Set the position in the buffer
        /// @param newPosition The new position to set
        /// @throws std::out_of_range If the new position is out of range
        void position(std::size_t newPosition) override;

        /// @brief Get the limit of the buffer
        /// @return The current limit
        [[nodiscard]] std::size_t limit() const  override;

        /// @brief Set the limit of the buffer
        /// @param newLimit The new limit to set
        /// @throws std::out_of_range If the new limit is out of range
        void limit(std::size_t newLimit) override;

        /// @brief Get the capacity of the buffer
        /// @return The capacity
        [[nodiscard]] std::size_t capacity() const  override;

        /// @brief Reset the buffer position to zero and set limit to capacity
        void clear()  override;

        /// @brief Flip the buffer (limit = position, position = 0)
        void flip()  override;

        /// @brief Reset the buffer position to zero
        void rewind()  override;

    private:
        std::vector<int64_t> buffer_{};
    };
}
