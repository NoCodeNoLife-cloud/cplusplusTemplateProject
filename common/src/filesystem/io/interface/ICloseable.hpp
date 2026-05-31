/**
* @file ICloseable.hpp
 * @brief ICloseable class declaration
 * @details This header defines the ICloseable class that provides functionality for Common library utilities.
 */

#pragma once

namespace common::interfaces
{
    /// @brief Interface for resources that can be closed.
    /// This interface defines a common contract for closing resources such as files,
    /// streams, or connections. Implementing classes must provide a mechanism to
    /// properly release or close the associated resource.
    class ICloseable
    {
    public:
        /// @brief Virtual destructor to ensure proper cleanup of derived classes
        virtual ~ICloseable() = default;

        /// @brief Close the resource.
        ///
        /// This method should release any system resources held by the implementing class.
        /// After calling this method, the resource should no longer be usable.
        /// Implementations should be idempotent - calling close multiple times should
        /// have the same effect as calling it once.
        virtual void close() = 0;

        /// @brief Check if the resource is closed.
        /// This method returns the current state of the resource.
        /// @return true if the resource is closed, false otherwise
        [[nodiscard]] virtual bool isClosed() const = 0;

        /// @brief Close the resource with exception handling.
        /// This method attempts to close the resource and handles any exceptions that may occur.
        /// If an exception occurs during closing, it will be caught and the method will return false.
        /// @return true if the resource was successfully closed, false otherwise
        [[nodiscard]] virtual bool closeSafe() ;
    };

    inline bool ICloseable::closeSafe()
    {
        try
        {
            close();
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}