#pragma once
#include <format>
#include <vector>

#include "src/interface/ICloseable.hpp"
#include "src/interface/IFlushable.hpp"

namespace common::filesystem {
    /// @brief Abstract base class for all output stream implementations.
    /// @details This class provides the basic contract for writing data to an output stream.
    /// It defines pure virtual methods that must be implemented by concrete subclasses.
    /// The class also implements the ICloseable and IFlushable interfaces to provide
    /// standard close and flush functionality.
    class AbstractOutputStream : public interfaces::ICloseable, public interfaces::IFlushable {
    public:
        ~AbstractOutputStream() override = default;

        /// @brief Writes a single byte to the output stream.
        /// @param b The byte to be written.
        virtual void write(std::byte b) = 0;

        /// @brief Writes all bytes from the specified buffer to the output stream.
        /// @param buffer The buffer containing bytes to be written.
        virtual auto write(const std::vector<std::byte> &buffer) -> void;

        /// @brief Writes a specified number of bytes from the buffer starting at the given offset to the output stream.
        /// @param buffer The buffer containing bytes to be written.
        /// @param offset The start offset in the buffer.
        /// @param len The number of bytes to write.
        /// @throws std::out_of_range if offset + len exceeds buffer size.
        virtual auto write(const std::vector<std::byte> &buffer, size_t offset, size_t len) -> void;

        /// @brief Writes a specified number of bytes from the buffer to the output stream.
        /// @param buffer The buffer containing bytes to be written.
        /// @param length The number of bytes to write.
        virtual auto write(const std::byte *buffer, size_t length) -> void = 0;

        /// @brief Flushes the output stream.
        auto flush() -> void override = 0;

        /// @brief Closes the output stream.
        auto close() -> void override = 0;
    };
}
