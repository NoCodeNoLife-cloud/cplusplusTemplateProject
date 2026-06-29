/**
 * @file DoubleBuffer.hpp
 * @brief NIO-style double-precision buffer with position/limit/capacity
 * @details A buffer for double data following the java.nio.Buffer pattern:
 *          maintains position, limit, and capacity.  Supports get/put,
 *          flip/clear/rewind, compact, and bulk transfer.  Backed by a
 *          std::vector<double>.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <stdexcept>
#include <vector>

#include <cppforge/interface/io/IBuffer.hpp>

namespace cppforge::buffer
{
    /// @brief A DoubleBuffer class that implements the IBuffer interface for handling double precision floating point
    /// numbers. This class provides functionality to put and get double values from a buffer with a specified capacity.
    class DoubleBuffer final : public interface::io::IBuffer
    {
    public:
        /// @brief Constructs a DoubleBuffer with the specified capacity.
        /// @param capacity The capacity of the buffer.
        explicit DoubleBuffer(size_t capacity);

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

        /// @brief Puts a single double value into the buffer.
        /// @param value The double value to be put into the buffer.
        /// @return Reference to the current DoubleBuffer object.
        DoubleBuffer& put(double value);

        /// @brief Puts a vector of double values into the buffer.
        /// @param values The vector of double values to be put into the buffer.
        /// @return Reference to the current DoubleBuffer object.
        DoubleBuffer& put(const std::vector<double>& values);

        /// @brief Gets a single double value from the buffer.
        /// @return The double value retrieved from the buffer.
        [[nodiscard]] double get();

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

        /// @brief Gets the remaining unprocessed elements as a vector.
        /// @return The remaining elements in the buffer as a vector.
        [[nodiscard]] std::vector<double> getRemaining() const;

    private:
        std::vector<double> buffer_{};
    };
}
