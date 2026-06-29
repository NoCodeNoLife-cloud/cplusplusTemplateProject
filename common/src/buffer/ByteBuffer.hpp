/**
 * @file ByteBuffer.hpp
 * @brief NIO-style byte buffer with position/limit/capacity tracking
 * @details A buffer for byte data following the java.nio.Buffer pattern:
 *          maintains position, limit, and capacity state.  Supports get/put
 *          operations (absolute and relative), flip/clear/rewind, compact,
 *          and bulk transfer.  Backed by a std::vector<std::byte>.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Usage Example
 * @code
 * ByteBuffer buf(16);
 * buf.put(std::byte{0x41});
 * buf.flip();
 * auto b = buf.get();  // 0x41
 * @endcode
 */

#pragma once
#include <vector>

#include "interface/io/IBuffer.hpp"

namespace cppforge::buffer
{
    /// @brief A byte buffer implementation that manages a fixed-size buffer of bytes.
    /// This class provides methods for putting and getting bytes, supporting both
    /// single-byte operations and bulk operations with vectors of bytes.
    class ByteBuffer final : public interface::io::IBuffer
    {
    public:
        explicit ByteBuffer(size_t capacity);

        /// @brief Get the capacity of the buffer
        /// @return The capacity of the buffer
        [[nodiscard]] size_t capacity() const  override;

        /// @brief Get the current position of the buffer
        /// @return The current position of the buffer
        [[nodiscard]] size_t position() const  override;

        /// @brief Set the position of the buffer
        /// @param newPosition The new position to set
        /// @throws std::out_of_range If newPosition exceeds the current limit
        void position(size_t newPosition) override;

        /// @brief Get the limit of the buffer
        /// @return The limit of the buffer
        [[nodiscard]] size_t limit() const  override;

        /// @brief Set the limit of the buffer
        /// @param newLimit The new limit to set
        /// @throws std::out_of_range If newLimit exceeds capacity
        void limit(size_t newLimit) override;

        /// @brief Clear the buffer by resetting position and limit
        void clear()  override;

        /// @brief Flip the buffer by setting limit to current position and resetting position to zero
        void flip()  override;

        /// @brief Rewind the buffer by resetting position to zero
        void rewind()  override;

        /// @brief Get the number of remaining elements in the buffer
        /// @return The number of remaining elements
        [[nodiscard]] size_t remaining() const  override;

        /// @brief Check if there are remaining elements in the buffer
        /// @return True if there are remaining elements, false otherwise
        [[nodiscard]] bool hasRemaining() const  override;

        /// @brief Compacts the buffer by removing processed elements
        void compact();

        /// @brief Gets the remaining unprocessed elements as a vector
        /// @return The remaining elements in the buffer as a vector
        [[nodiscard]] std::vector<std::byte> getRemaining() const;

        /// @brief Puts a single byte into the buffer
        /// @param value The byte value to put into the buffer
        /// @throws std::overflow_error If buffer has no remaining space
        void put(std::byte value);

        /// @brief Puts bytes from a vector into the buffer
        /// @param src The source vector of bytes to put into the buffer
        /// @throws std::overflow_error If buffer has insufficient space
        void put(const std::vector<std::byte>& src);

        /// @brief Gets a single byte from the buffer
        /// @return The byte retrieved from the buffer
        /// @throws std::underflow_error If buffer has no remaining data
        [[nodiscard]] std::byte get();

        /// @brief Gets a specified number of bytes from the buffer
        /// @param length The number of bytes to retrieve
        /// @return A vector containing the retrieved bytes
        /// @throws std::underflow_error If buffer has insufficient data
        [[nodiscard]] std::vector<std::byte> get(size_t length);

    private:
        std::vector<std::byte> buffer_;
    };
}
