/**
 * @file IReadable.hpp
 * @brief IReadable class declaration
 * @details This header defines the IReadable class that provides functionality for Common interface definitions for the framework.
 */

#pragma once

namespace common::interfaces
{
    /// @brief Interface for readable objects.
    /// This interface defines the contract for objects that can be read from.
    /// Implementing classes should provide functionality to read data, typically byte by byte.
    class IReadable
    {
    public:
        /// @brief Virtual destructor to ensure proper cleanup of derived classes
        virtual ~IReadable() = default;

        /// @brief Read one byte from the input source.
        /// This method reads and returns the next byte of data from the input source.
        /// @return The byte value read as an unsigned integer, or -1 if the end of the stream has been reached
        [[nodiscard]] virtual int read() = 0;
    };
}
