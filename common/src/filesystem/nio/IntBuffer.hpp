#pragma once
#include <stdexcept>
#include <vector>

#include "src/interface/IBuffer.hpp"

namespace common::filesystem {
    /// @brief A buffer that stores 32-bit integers.
    /// @details This class provides a way to store and manipulate a sequence of 32-bit integers in a buffer.
    /// It implements the IBuffer interface and provides methods to get and put integers in the buffer.
    class IntBuffer final : public interfaces::IBuffer {
    public:
        /// @brief Constructs an IntBuffer with the specified capacity.
        /// @param capacity The capacity of the buffer.
        explicit IntBuffer(size_t capacity);

        /// @brief Clears the buffer by setting position to 0 and limit to capacity.
        /// @return void
        auto clear() noexcept -> void override;

        /// @brief Flips the buffer by setting limit to current position and position to 0.
        /// @return void
        auto flip() noexcept -> void override;

        /// @brief Resets the position to 0 while keeping the limit unchanged.
        /// @return void
        auto rewind() noexcept -> void override;

        /// @brief Compacts the buffer by removing processed elements.
        /// @return void
        auto compact() -> void;

        /// @brief Get the current element and advance the position.
        /// @return The current element.
        [[nodiscard]] auto get() -> int32_t;

        /// @brief Get the element at the specified index.
        /// @param index The index of the element to get.
        /// @return The element at the specified index.
        [[nodiscard]] auto get(size_t index) const -> int32_t;

        /// @brief Put the specified value at the current position and advance the position.
        /// @param value The value to put.
        /// @return void
        auto put(int32_t value) -> void;

        /// @brief Put the specified value at the specified index.
        /// @param index The index at which to put the value.
        /// @param value The value to put.
        /// @return void
        auto put(size_t index, int32_t value) -> void;

        /// @brief Gets the remaining unprocessed elements as a vector.
        /// @return The remaining elements in the buffer as a vector.
        [[nodiscard]] auto getRemaining() const -> std::vector<int32_t>;

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
        std::vector<int32_t> buffer_{};
    };
}
