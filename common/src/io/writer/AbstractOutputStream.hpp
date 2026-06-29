/**
 * @file AbstractOutputStream.hpp
 * @brief Abstract base class for byte-output streams
 * @details Defines the contract for writing byte data to an output sink.
 *          Implements ICloseable and IFlushable with pure-virtual write()
 *          methods.  Provides convenience overloads that delegate to the
 *          core write(std::byte) and write(buffer, length) operations.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  Derived classes may add thread-safety
 * guarantees.
 *
 * @par Design
 * Follows the Java OutputStream abstraction pattern: subclasses implement
 * write(std::byte) and write(buffer, length), and the base class provides
 * default offset/length logic for the vector overloads.
 */

#pragma once
#include <cstddef>
#include <vector>

#include "interface/io/ICloseable.hpp"
#include "interface/io/IFlushable.hpp"

namespace cppforge::io::writer
{
    /// @brief Abstract base class for all output stream implementations.
    /// @details This class provides the basic contract for writing data to an output stream.
    /// It defines pure virtual methods that must be implemented by concrete subclasses.
    /// The class also implements the ICloseable and IFlushable interfaces to provide
    /// standard close and flush functionality.
    class AbstractOutputStream : public interface::io::ICloseable, public interface::io::IFlushable
    {
    public:
        ~AbstractOutputStream() override = default;

        /// @brief Writes a single byte to the output stream.
        /// @param b The byte to be written.
        virtual void write(std::byte b) = 0;

        /// @brief Writes all bytes from the specified buffer to the output stream.
        /// @param buffer The buffer containing bytes to be written.
        virtual void write(const std::vector<std::byte>& buffer);

        /// @brief Writes a specified number of bytes from the buffer starting at the given offset to the output stream.
        /// @param buffer The buffer containing bytes to be written.
        /// @param offset The start offset in the buffer.
        /// @param len The number of bytes to write.
        /// @throws std::out_of_range if offset + len exceeds buffer size.
        virtual void write(const std::vector<std::byte>& buffer, size_t offset, size_t len);

        /// @brief Writes a specified number of bytes from the buffer to the output stream.
        /// @param buffer The buffer containing bytes to be written.
        /// @param length The number of bytes to write.
        virtual void write(const std::byte* buffer, size_t length) = 0;

        /// @brief Flushes the output stream.
        void flush() override = 0;

        /// @brief Closes the output stream.
        void close() override = 0;
    };
}
