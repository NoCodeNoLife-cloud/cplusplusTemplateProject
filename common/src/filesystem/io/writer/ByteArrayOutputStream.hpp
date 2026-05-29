/**
 * @file ByteArrayOutputStream.hpp
 * @brief ByteArrayOutputStream class declaration
 * @details This header defines the ByteArrayOutputStream class that provides functionality for Common library utilities.
 */

#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>

#include "src/filesystem/io/writer/AbstractOutputStream.hpp"

namespace common::filesystem
{
    /// @brief A ByteArrayOutputStream is an output stream that writes data into a byte array.
    /// The buffer automatically grows as data is written to it.
    class ByteArrayOutputStream final : public AbstractOutputStream
    {
    public:
        /// @brief Constructs a new byte array output stream with default buffer size.
        ByteArrayOutputStream();

        /// @brief Constructs a new byte array output stream with the specified initial size.
        /// @param size The initial size of the buffer.
        /// @throws std::invalid_argument if size is zero.
        explicit ByteArrayOutputStream(size_t size);

        /// @brief Writes a single byte to the stream.
        /// @param b The byte to write.
        auto write(std::byte b) -> void override;

        /// @brief Writes a sequence of bytes to the stream from the specified buffer.
        /// @param buffer The buffer containing bytes to write.
        /// @param offset The start offset in the buffer.
        /// @param len The number of bytes to write.
        /// @throws std::out_of_range if offset and length are out of the bounds of the buffer.
        auto write(const std::vector<std::byte>& buffer, size_t offset, size_t len) -> void override;

        /// @brief Writes a sequence of bytes to the stream from the specified buffer.
        /// @param buffer The buffer containing bytes to write.
        /// @param length The number of bytes to write.
        /// @throws std::invalid_argument if buffer is null.
        auto write(const std::byte* buffer, size_t length) -> void override;

        /// @brief Writes the complete contents of this byte array output stream to the specified output stream.
        /// @param out The output stream to which to write the data.
        auto writeTo(AbstractOutputStream& out) const -> void;

        /// @brief Resets the count field of this byte array output stream to zero.
        auto reset() -> void;

        /// @brief Creates a newly allocated byte array.
        /// @return The current contents of this output stream, as a byte array.
        [[nodiscard]] auto toByteArray() const -> std::vector<std::byte>;

        /// @brief Returns the current size of the buffer.
        /// @return The value of the count field, which is the number of valid bytes in the buffer.
        [[nodiscard]] auto size() const -> size_t;

        /// @brief Converts the buffer's contents into a string by decoding the bytes using the platform's default charset.
        /// @return A string representation of the buffer's contents.
        [[nodiscard]] auto toString() const -> std::string;

        /// @brief Flushes this output stream and forces any buffered output bytes to be written out.
        auto flush() -> void override;

        /// @brief Closes this output stream and releases any system resources associated with this stream.
        auto close() -> void override;

        /// @brief Returns the current capacity of the buffer.
        /// @return The current capacity of the buffer.
        [[nodiscard]] auto capacity() const -> size_t;

        /// @brief Checks if the stream is closed.
        /// @return True if the stream is closed, false otherwise.
        [[nodiscard]] auto isClosed() const -> bool override;

    protected:
        std::vector<std::byte> buf_{};
        size_t count_{};

    private:
        /// @brief Ensures that the buffer has at least the specified capacity.
        /// @param additionalCapacity The additional capacity needed.
        auto ensureCapacity(size_t additionalCapacity) -> void;
    };
}
