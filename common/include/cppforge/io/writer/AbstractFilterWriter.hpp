/**
 * @file AbstractFilterWriter.hpp
 * @brief Decorator base class for character-output writers
 * @details Implements the decorator pattern for AbstractWriter.  Stores a
 *          pointer to an underlying AbstractWriter and delegates all
 *          write/append/flush/close calls to it.  Subclasses override specific
 *          methods to add buffering, filtering, or encoding transformations.
 *          Analogous to java.io.FilterWriter.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <memory>
#include <string>
#include <vector>

#include <cppforge/io/writer/AbstractWriter.hpp>

namespace cppforge::io::writer
{
    /// @brief Abstract base class for filter writers.
    /// This class provides a common interface for writers that filter or transform data before writing it to an underlying
    /// output writer. Subclasses should implement the specific filtering or transformation logic.
    class AbstractFilterWriter : public AbstractWriter
    {
    public:
        /// @brief Constructs a filter writer with the specified output writer.
        /// @param outputWriter The underlying writer to which filtered data will be written.
        /// @throws std::invalid_argument if outputWriter is null.
        explicit AbstractFilterWriter(std::unique_ptr<AbstractWriter> outputWriter);

        ~AbstractFilterWriter() override;

        /// @brief Writes a single character to the output writer.
        /// This method overrides the base class method to provide filtering functionality.
        /// The character is processed and then passed to the underlying output writer.
        /// @param c The character to write.
        void write(char c) override;

        /// @brief Writes a sequence of characters from a vector to the output writer.
        /// This method overrides the base class method to provide filtering functionality.
        /// The characters are processed and then passed to the underlying output writer.
        /// @param cBuf The vector containing the characters to write.
        /// @param off The starting offset in the vector.
        /// @param len The number of characters to write.
        /// @throws std::out_of_range if off + len exceeds the buffer size.
        void write(const std::vector<char>& cBuf, size_t off, size_t len) override;

        /// @brief Writes a sequence of characters from a vector to the output writer.
        /// This method overrides the base class method to provide filtering functionality.
        /// The characters are processed and then passed to the underlying output writer.
        /// @param cBuf The vector containing the characters to write.
        void write(const std::vector<char>& cBuf) override;

        /// @brief Writes a sequence of characters from a string to the output writer.
        /// This method overrides the base class method to provide filtering functionality.
        /// The characters are processed and then passed to the underlying output writer.
        /// @param str The string containing the characters to write.
        /// @param off The starting offset in the string.
        /// @param len The number of characters to write.
        /// @throws std::out_of_range if off + len exceeds the string size.
        void write(const std::string& str, size_t off, size_t len) override;

        /// @brief Writes a sequence of characters from a string to the output writer.
        /// This method overrides the base class method to provide filtering functionality.
        /// The characters are processed and then passed to the underlying output writer.
        /// @param str The string containing the characters to write.
        void write(const std::string& str) override;

        /// @brief Flushes the output writer.
        /// This method overrides the base class method to provide filtering functionality.
        /// It ensures that any buffered data is written to the underlying output writer.
        void flush() override;

        /// @brief Closes the output writer.
        /// This method overrides the base class method to provide filtering functionality.
        /// It ensures that the underlying output writer is also closed.
        void close() override;

    protected:
        std::unique_ptr<AbstractWriter> output_writer_;

    private:
        /// @brief Checks if the output stream is available.
        /// @throws std::runtime_error if the output stream is not available.
        void checkOutputStream() const;
    };
}
