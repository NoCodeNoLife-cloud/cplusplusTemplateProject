/**
 * @file ByteArrayOutputStream.hpp
 * @brief ByteArrayOutputStream class declaration
 * @details This header defines the ByteArrayOutputStream class that provides functionality for Common library utilities.
 */

#pragma once
#include <algorithm>
#include <cstddef>
#include <vector>

#include "filesystem/io/writer/AbstractOutputStream.hpp"

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
        void write(std::byte b) override;

        /// @brief Writes a sequence of bytes to the stream from the specified buffer.
        /// @param buffer The buffer containing bytes to write.
        /// @param offset The start offset in the buffer.
        /// @param len The number of bytes to write.
        /// @throws std::out_of_range if offset and length are out of the bounds of the buffer.
        void write(const std::vector<std::byte>& buffer, size_t offset, size_t len) override;

        /// @brief Writes a sequence of bytes to the stream from the specified buffer.
        /// @param buffer The buffer containing bytes to write.
        /// @param length The number of bytes to write.
        /// @throws std::invalid_argument if buffer is null.
        void write(const std::byte* buffer, size_t length) override;

        /// @brief Writes the complete contents of this byte array output stream to the specified output stream.
        /// @param out The output stream to which to write the data.
        void writeTo(AbstractOutputStream& out) const;

        /// @brief Resets the count field of this byte array output stream to zero.
        void reset();

        /// @brief Creates a newly allocated byte array.
        /// @return The current contents of this output stream, as a byte array.
        [[nodiscard]] std::vector<std::byte> toByteArray() const;

        /// @brief Returns the current size of the buffer.
        /// @return The value of the count field, which is the number of valid bytes in the buffer.
        [[nodiscard]] size_t size() const;

        /// @brief Converts the buffer's contents into a string by decoding the bytes using the platform's default charset.
        /// @return A string representation of the buffer's contents.
        [[nodiscard]] std::string toString() const;

        /// @brief Flushes this output stream and forces any buffered output bytes to be written out.
        void flush() override;

        /// @brief Closes this output stream and releases any system resources associated with this stream.
        void close() override;

        /// @brief Returns the current capacity of the buffer.
        /// @return The current capacity of the buffer.
        [[nodiscard]] size_t capacity() const;

        /// @brief Checks if the stream is closed.
        /// @return True if the stream is closed, false otherwise.
        [[nodiscard]] bool isClosed() const override;

    protected:
        std::vector<std::byte> buf_{};
        size_t count_{};

    private:
        /// @brief Ensures that the buffer has at least the specified capacity.
        /// @param additionalCapacity The additional capacity needed.
        void ensureCapacity(size_t additionalCapacity);
    };
}