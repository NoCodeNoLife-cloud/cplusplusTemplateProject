#pragma once
#include <stdexcept>
#include <vector>

#include "src/interface/IBuffer.hpp"

namespace common::filesystem {
    /// @brief A DoubleBuffer class that implements the IBuffer interface for handling double precision floating point
    /// numbers. This class provides functionality to put and get double values from a buffer with a specified capacity.
    class DoubleBuffer final : public interfaces::IBuffer {
    public:
        /// @brief Constructs a DoubleBuffer with the specified capacity.
        /// @param capacity The capacity of the buffer.
        explicit DoubleBuffer(size_t capacity);

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

        /// @brief Puts a single double value into the buffer.
        /// @param value The double value to be put into the buffer.
        /// @return Reference to the current DoubleBuffer object.
        auto put(double value) -> DoubleBuffer &;

        /// @brief Puts a vector of double values into the buffer.
        /// @param values The vector of double values to be put into the buffer.
        /// @return Reference to the current DoubleBuffer object.
        auto put(const std::vector<double> &values) -> DoubleBuffer &;

        /// @brief Gets a single double value from the buffer.
        /// @return The double value retrieved from the buffer.
        [[nodiscard]] auto get() -> double;

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

        /// @brief Gets the remaining unprocessed elements as a vector.
        /// @return The remaining elements in the buffer as a vector.
        [[nodiscard]] auto getRemaining() const -> std::vector<double>;

    private:
        std::vector<double> buffer_{};
    };
}
