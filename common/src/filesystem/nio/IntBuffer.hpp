/**
 * @file IntBuffer.hpp
 * @brief IntBuffer class declaration
 * @details This header defines the IntBuffer class that provides functionality for Common library utilities.
 */

#pragma once
#include <stdexcept>
#include <vector>

#include "interface/io/IBuffer.hpp"

namespace common::filesystem
{
    /// @brief A buffer that stores 32-bit integers.
    /// @details This class provides a way to store and manipulate a sequence of 32-bit integers in a buffer.
    /// It implements the IBuffer interface and provides methods to get and put integers in the buffer.
    class IntBuffer final : public interfaces::io::IBuffer
    {
    public:
        /// @brief Constructs an IntBuffer with the specified capacity.
        /// @param capacity The capacity of the buffer.
        explicit IntBuffer(size_t capacity);

        /// @brief Clears the buffer by setting position to 0 and limit to capacity.
        /// @return void
        void clear()  override;

        /// @brief Flips the buffer by setting limit to current position and position to 0.
        /// @return void
        void flip()  override;

        /// @brief Resets the position to 0 while keeping the limit unchanged.
        /// @return void
        void rewind()  override;

        /// @brief Compacts the buffer by removing processed elements.
        /// @return void
        void compact();

        /// @brief Get the current element and advance the position.
        /// @return The current element.
        [[nodiscard]] int32_t get();

        /// @brief Get the element at the specified index.
        /// @param index The index of the element to get.
        /// @return The element at the specified index.
        [[nodiscard]] int32_t get(size_t index) const;

        /// @brief Put the specified value at the current position and advance the position.
        /// @param value The value to put.
        /// @return void
        void put(int32_t value);

        /// @brief Put the specified value at the specified index.
        /// @param index The index at which to put the value.
        /// @param value The value to put.
        /// @return void
        void put(size_t index, int32_t value);

        /// @brief Gets the remaining unprocessed elements as a vector.
        /// @return The remaining elements in the buffer as a vector.
        [[nodiscard]] std::vector<int32_t> getRemaining() const;

        /// @brief Returns the current position in the buffer.
        /// @return The current position.
        [[nodiscard]] size_t position() const  override;

        /// @brief Sets the position in the buffer.
        /// @param newPosition The new position to set.
        /// @return void
        void position(size_t newPosition) override;

        /// @brief Returns the limit of the buffer.
        /// @return The current limit.
        [[nodiscard]] size_t limit() const  override;

        /// @brief Sets the limit of the buffer.
        /// @param newLimit The new limit to set.
        /// @return void
        void limit(size_t newLimit) override;

        /// @brief Returns the capacity of the buffer.
        /// @return The buffer capacity.
        [[nodiscard]] size_t capacity() const  override;

        /// @brief Checks if there are remaining elements in the buffer.
        /// @return true if there are remaining elements, false otherwise.
        [[nodiscard]] bool hasRemaining() const  override;

        /// @brief Returns the number of remaining elements in the buffer.
        /// @return The number of remaining elements.
        [[nodiscard]] size_t remaining() const  override;

    private:
        std::vector<int32_t> buffer_{};
    };
}
