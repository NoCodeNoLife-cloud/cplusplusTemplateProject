/**
 * @file ShortBuffer.hpp
 * @brief ShortBuffer class declaration
 * @details This header defines the ShortBuffer class that provides functionality for Common library utilities.
 */

#pragma once
#include <cstring>
#include <vector>

#include "interface/IBuffer.hpp"

namespace common::filesystem
{
    /// @brief A buffer for short integer (int16_t) data.
    /// ShortBuffer provides a way to store, read, and write short integer data in a
    /// sequential manner. It implements the IBuffer interface and supports operations
    /// like get, put, and rewind.
    class ShortBuffer final : public interfaces::IBuffer
    {
    public:
        /// @brief Construct a ShortBuffer with the specified capacity
        /// @param capacity The initial capacity of the buffer
        explicit ShortBuffer(size_t capacity);

        /// @brief Wraps an existing array of int16_t data into a ShortBuffer.
        /// @param data Pointer to the data to wrap.
        /// @param size Size of the data array.
        /// @return A new ShortBuffer instance wrapping the provided data.
        static ShortBuffer wrap(const int16_t* data, size_t size);

        /// @brief Reads the next int16_t value from the buffer.
        /// @return The next int16_t value.
        /// @throws std::underflow_error If no remaining elements to get
        [[nodiscard]] int16_t get();

        /// @brief Reads an int16_t value at the specified index.
        /// @param index Index of the value to read.
        /// @return The int16_t value at the specified index.
        /// @throws std::out_of_range If index is out of bounds
        [[nodiscard]] int16_t get(size_t index) const;

        /// @brief Writes an int16_t value to the buffer at the current position.
        /// @param value The value to write.
        /// @throws std::overflow_error If no remaining space to put
        void put(int16_t value);

        /// @brief Writes an int16_t value to the buffer at the specified index.
        /// @param index Index where the value should be written.
        /// @param value The value to write.
        /// @throws std::out_of_range If index is out of bounds
        void put(size_t index, int16_t value);

        /// @brief Compacts the buffer by removing processed elements
        void compact();

        /// @brief Gets the remaining unprocessed elements as a vector
        /// @return The remaining elements in the buffer as a vector
        [[nodiscard]] std::vector<int16_t> getRemaining() const;

        /// @brief Check if there are remaining elements in the buffer
        /// @return True if there are remaining elements, false otherwise
        [[nodiscard]] bool hasRemaining() const  override;

        /// @brief Get the number of remaining elements in the buffer
        /// @return Number of remaining elements
        [[nodiscard]] size_t remaining() const  override;

        /// @brief Get the current position in the buffer
        /// @return The current position
        [[nodiscard]] size_t position() const  override;

        /// @brief Set the position in the buffer
        /// @param newPosition The new position to set
        /// @throws std::out_of_range If the new position is out of range
        void position(size_t newPosition) override;

        /// @brief Get the limit of the buffer
        /// @return The current limit
        [[nodiscard]] size_t limit() const  override;

        /// @brief Set the limit of the buffer
        /// @param newLimit The new limit to set
        /// @throws std::out_of_range If the new limit is out of range
        void limit(size_t newLimit) override;

        /// @brief Get the capacity of the buffer
        /// @return The capacity
        [[nodiscard]] size_t capacity() const  override;

        /// @brief Reset the buffer position to zero and set limit to capacity
        void clear()  override;

        /// @brief Flip the buffer (limit = position, position = 0)
        void flip()  override;

        /// @brief Resets the position of the buffer to zero.
        void rewind()  override;

        /// @brief Returns a pointer to the underlying data array.
        /// @return Pointer to the data array.
        [[nodiscard]] int16_t* data();

        /// @brief Returns a const pointer to the underlying data array.
        /// @return Const pointer to the data array.
        [[nodiscard]] const int16_t* data() const;

    private:
        std::vector<int16_t> buffer_{};
    };
}
