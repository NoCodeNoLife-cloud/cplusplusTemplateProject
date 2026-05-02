#pragma once
#include <cstring>
#include <vector>

#include "src/interface/IBuffer.hpp"

namespace common::filesystem {
    /// @brief A buffer for short integer (int16_t) data.
    /// ShortBuffer provides a way to store, read, and write short integer data in a
    /// sequential manner. It implements the IBuffer interface and supports operations
    /// like get, put, and rewind.
    class ShortBuffer final : public interfaces::IBuffer {
    public:
        /// @brief Construct a ShortBuffer with the specified capacity
        /// @param capacity The initial capacity of the buffer
        explicit ShortBuffer(size_t capacity);

        /// @brief Wraps an existing array of int16_t data into a ShortBuffer.
        /// @param data Pointer to the data to wrap.
        /// @param size Size of the data array.
        /// @return A new ShortBuffer instance wrapping the provided data.
        static auto wrap(const int16_t *data, size_t size) -> ShortBuffer;

        /// @brief Reads the next int16_t value from the buffer.
        /// @return The next int16_t value.
        /// @throws std::out_of_range If no remaining elements to get
        [[nodiscard]] auto get() -> int16_t;

        /// @brief Reads an int16_t value at the specified index.
        /// @param index Index of the value to read.
        /// @return The int16_t value at the specified index.
        /// @throws std::out_of_range If index is out of bounds
        [[nodiscard]] auto get(size_t index) const -> int16_t;

        /// @brief Writes an int16_t value to the buffer at the current position.
        /// @param value The value to write.
        /// @throws std::out_of_range If no remaining space to put
        auto put(int16_t value) -> void;

        /// @brief Writes an int16_t value to the buffer at the specified index.
        /// @param index Index where the value should be written.
        /// @param value The value to write.
        /// @throws std::out_of_range If index is out of bounds
        auto put(size_t index, int16_t value) -> void;

        /// @brief Check if there are remaining elements in the buffer
        /// @return True if there are remaining elements, false otherwise
        [[nodiscard]] auto hasRemaining() const noexcept -> bool override;

        /// @brief Get the number of remaining elements in the buffer
        /// @return Number of remaining elements
        [[nodiscard]] auto remaining() const noexcept -> size_t override;

        /// @brief Get the current position in the buffer
        /// @return The current position
        [[nodiscard]] auto position() const noexcept -> size_t override;

        /// @brief Set the position in the buffer
        /// @param newPosition The new position to set
        /// @throws std::out_of_range If the new position is out of range
        auto position(size_t newPosition) -> void override;

        /// @brief Get the limit of the buffer
        /// @return The current limit
        [[nodiscard]] auto limit() const noexcept -> size_t override;

        /// @brief Set the limit of the buffer
        /// @param newLimit The new limit to set
        /// @throws std::out_of_range If the new limit is out of range
        auto limit(size_t newLimit) -> void override;

        /// @brief Get the capacity of the buffer
        /// @return The capacity
        [[nodiscard]] auto capacity() const noexcept -> size_t override;

        /// @brief Reset the buffer position to zero and set limit to capacity
        auto clear() noexcept -> void override;

        /// @brief Flip the buffer (limit = position, position = 0)
        auto flip() noexcept -> void override;

        /// @brief Resets the position of the buffer to zero.
        auto rewind() noexcept -> void override;

        /// @brief Returns a pointer to the underlying data array.
        /// @return Pointer to the data array.
        [[nodiscard]] auto data() -> int16_t *;

        /// @brief Returns a const pointer to the underlying data array.
        /// @return Const pointer to the data array.
        [[nodiscard]] auto data() const -> const int16_t *;

    private:
        std::vector<int16_t> buffer_{};
        size_t capacity_{};
        size_t limit_{};
        size_t position_{};
    };
}
