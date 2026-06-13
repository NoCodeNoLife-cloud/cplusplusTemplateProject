/**
 * @file FloatBuffer.hpp
 * @brief NIO-style float buffer with position/limit/capacity tracking
 * @details A buffer for float data following the java.nio.Buffer pattern:
 *          maintains position, limit, and capacity.  Supports get/put,
 *          flip/clear/rewind, compact, and bulk transfer.  Backed by a
 *          std::vector<float>.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <vector>

#include "interface/io/IBuffer.hpp"

namespace common::buffer
{
    /// @brief A buffer that stores float values.
    /// @details This class provides methods to put and get float values from a buffer.
    /// It is designed to be used in a network I/O context.
    class FloatBuffer final : public interface::io::IBuffer
    {
    public:
        /// @brief Constructs a FloatBuffer with the specified capacity.
        /// @param capacity The capacity of the buffer.
        explicit FloatBuffer(size_t capacity);

        /// @brief Allocates a new FloatBuffer with the specified capacity.
        /// @param capacity The capacity of the buffer.
        /// @return A new FloatBuffer instance.
        static FloatBuffer allocate(size_t capacity);

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

        /// @brief Puts a single float value into the buffer.
        /// @param value The float value to put.
        /// @return void
        void put(float value);

        /// @brief Puts a vector of float values into the buffer.
        /// @param values The vector of float values to put.
        /// @return void
        void put(const std::vector<float>& values);

        /// @brief Gets a single float value from the buffer.
        /// @return The float value retrieved.
        [[nodiscard]] float get();

        /// @brief Gets a vector of float values from the buffer.
        /// @param length The number of elements to retrieve.
        /// @return A vector containing the retrieved float values.
        [[nodiscard]] std::vector<float> get(size_t length);

        /// @brief Gets the remaining unprocessed elements as a vector.
        /// @return The remaining elements in the buffer as a vector.
        [[nodiscard]] std::vector<float> getRemaining() const;

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
        std::vector<float> buffer_{};
    };
}
