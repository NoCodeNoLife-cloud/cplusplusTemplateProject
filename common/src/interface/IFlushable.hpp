#pragma once

namespace common::interfaces {
    /// @brief Interface for flushable objects.
    /// This interface defines a contract for objects that can be flushed,
    /// typically used for streams or buffers that need to ensure all data is written out.
    class IFlushable {
    public:
        /// @brief Virtual destructor to ensure proper cleanup of derived classes
        virtual ~IFlushable() = default;

        /// @brief Flush the stream or buffer.
        /// This method ensures that any buffered data is written out to the underlying destination.
        /// Implementation should handle any errors that may occur during the flush operation.
        /// If an error occurs, the implementation may throw an exception.
        virtual auto flush() -> void = 0;

        /// @brief Flush the stream or buffer with exception safety.
        /// This method ensures that any buffered data is written out to the underlying destination
        /// and handles any exceptions that may occur during the flush operation.
        /// @return true if flush was successful, false otherwise
        [[nodiscard]] virtual auto flushSafe() noexcept -> bool;

        /// @brief Check if the object needs to be flushed.
        /// This method checks if there is buffered data that needs to be flushed.
        /// The default implementation returns true, indicating that flush is always needed.
        /// Derived classes can override this method based on actual conditions.
        /// @return true if there is buffered data that needs to be flushed, false otherwise
        [[nodiscard]] virtual auto isFlushNeeded() const -> bool;

    protected:
        /// @brief Protected constructor to prevent direct instantiation of the interface
        IFlushable() = default;
    };

    inline auto IFlushable::flushSafe() noexcept -> bool {
        try {
            flush();
            return true;
        } catch (...) {
            return false;
        }
    }

    inline auto IFlushable::isFlushNeeded() const -> bool {
        // Default implementation returns true, indicating that flush is always needed
        // Derived classes can override this method based on actual conditions
        return true;
    }
}
