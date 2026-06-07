/**
 * @file CharBuffer.hpp
 * @brief CharBuffer class declaration
 * @details This header defines the CharBuffer class that provides functionality for Common library utilities.
 */

#pragma once
#include <algorithm>
#include <string>

#include "interface/io/IBuffer.hpp"

namespace common::filesystem::nio
{
    /// @brief A character buffer that implements the IBuffer interface.
    /// @details This class provides functionality for managing a buffer of characters,
    ///          including operations such as putting and getting characters, compacting
    ///          the buffer, and retrieving remaining unprocessed characters.
    class CharBuffer final : public interface::io::IBuffer
    {
    public:
        /// @brief Constructs a CharBuffer with the specified capacity.
        /// @param cap The capacity of the buffer.
        explicit CharBuffer(size_t cap);

        /// @brief Clears the buffer by setting position to 0 and limit to capacity.
        /// @return void
        void clear()  override;

        /// @brief Flips the buffer by setting limit to current position and position to 0.
        /// @return void
        void flip()  override;

        /// @brief Resets the position to 0 while keeping the limit unchanged.
        /// @return void
        void rewind()  override;

        /// @brief Compacts the buffer by removing processed characters.
        /// @return void
        void compact();

        /// @brief Puts a single character into the buffer.
        /// @param c The character to put into the buffer.
        /// @return void
        void put(char c);

        /// @brief Puts a string into the buffer.
        /// @param src The string to put into the buffer.
        /// @return void
        void put(const std::string& src);

        /// @brief Gets a single character from the buffer.
        /// @return The character retrieved from the buffer.
        [[nodiscard]] char get();

        /// @brief Gets the remaining unprocessed characters as a string.
        /// @return The remaining characters in the buffer as a string.
        [[nodiscard]] std::string getRemaining() const;

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
        std::string buffer_{};
    };
}
