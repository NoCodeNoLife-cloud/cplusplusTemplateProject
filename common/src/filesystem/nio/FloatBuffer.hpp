#pragma once
#include <vector>

#include "src/interface/IBuffer.hpp"

namespace common::filesystem {
    /// @brief A buffer that stores float values.
    /// @details This class provides methods to put and get float values from a buffer.
    /// It is designed to be used in a network I/O context.
    class FloatBuffer final : public interfaces::IBuffer {
    public:
        /// @brief Constructs a FloatBuffer with the specified capacity.
        /// @param capacity The capacity of the buffer.
        explicit FloatBuffer(size_t capacity);

        /// @brief Allocates a new FloatBuffer with the specified capacity.
        /// @param capacity The capacity of the buffer.
        /// @return A new FloatBuffer instance.
        static auto allocate(size_t capacity) -> FloatBuffer;

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

        /// @brief Puts a single float value into the buffer.
        /// @param value The float value to put.
        /// @return void
        auto put(float value) -> void;

        /// @brief Puts a vector of float values into the buffer.
        /// @param values The vector of float values to put.
        /// @return void
        auto put(const std::vector<float> &values) -> void;

        /// @brief Gets a single float value from the buffer.
        /// @return The float value retrieved.
        [[nodiscard]] auto get() -> float;

        /// @brief Gets a vector of float values from the buffer.
        /// @param length The number of elements to retrieve.
        /// @return A vector containing the retrieved float values.
        [[nodiscard]] auto get(size_t length) -> std::vector<float>;

        /// @brief Gets the remaining unprocessed elements as a vector.
        /// @return The remaining elements in the buffer as a vector.
        [[nodiscard]] auto getRemaining() const -> std::vector<float>;

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
        std::vector<float> buffer_{};
    };
}
