#pragma once
#include <algorithm>
#include <vector>
#include <string>
#include <string_view>

#include "AbstractWriter.hpp"

namespace common::filesystem {
/// @brief A class for writing character data to a buffer in memory.
/// CharArrayWriter is a utility class that allows you to write character data
/// to an internal buffer. The buffer automatically expands as needed to hold
/// more data. You can retrieve the contents of the buffer as either a string
/// or a character array.
class CharArrayWriter final : public AbstractWriter, interfaces::IAppendable<CharArrayWriter> {
public:
    /// @brief Constructs a new character array writer with default buffer size.
    CharArrayWriter() = default;

    /// @brief Constructs a new character array writer with the specified initial size.
    /// @param initialSize The initial size of the buffer.
    /// @throws std::invalid_argument if initialSize is negative.
    explicit CharArrayWriter(int32_t initialSize);

    /// @brief Destructor for CharArrayWriter.
    ~CharArrayWriter() override = default;

    // Writer functions
    /// @brief Writes a single character to the buffer.
    /// @param c The character to write.
    auto write(char c) -> void override;

    /// @brief Writes a portion of a character array to the buffer.
    /// @param cBuf The character array to write from.
    /// @param off The offset in the array to start writing from.
    /// @param len The number of characters to write.
    /// @throws std::out_of_range if offset and length are out of the bounds of the buffer.
    auto write(const std::vector<char>& cBuf, size_t off, size_t len) -> void override;

    /// @brief Writes a character array to the buffer.
    /// @param cBuf The character array to write.
    auto write(const std::vector<char>& cBuf) -> void override;

    /// @brief Writes a portion of a string to the buffer.
    /// @param str The string to write from.
    /// @param off The offset in the string to start writing from.
    /// @param len The number of characters to write.
    /// @throws std::out_of_range if offset and length are out of the bounds of the string.
    auto write(const std::string& str, size_t off, size_t len) -> void override;

    /// @brief Writes a string to the buffer.
    /// @param str The string to write.
    auto write(const std::string& str) -> void override;

    /// @brief Writes the contents of this writer to another writer.
    /// @param out The writer to write to.
    auto writeTo(AbstractWriter& out) const -> void;

    // Appendable functions
    /// @brief Appends a single character to the buffer.
    /// @param c The character to append.
    /// @return A reference to this CharArrayWriter instance.
    auto append(char c) -> CharArrayWriter& override;

    /// @brief Appends a string to the buffer.
    /// @param csq The string to append.
    /// @return A reference to this CharArrayWriter instance.
    auto append(const std::string& csq) -> CharArrayWriter& override;

    /// @brief Appends a subsequence of a string to the buffer.
    /// @param csq The string to append from.
    /// @param start The starting index of the subsequence.
    /// @param end The ending index of the subsequence.
    /// @return A reference to this CharArrayWriter instance.
    auto append(const std::string& csq, size_t start, size_t end) -> CharArrayWriter& override;

    /// @brief Appends a string view to the buffer.
    /// @param str The string view to append.
    /// @return A reference to this CharArrayWriter instance.
    auto append(std::string_view str) -> CharArrayWriter& override;

    /// @brief Appends a C-string to the buffer.
    /// @param str The C-string to append.
    /// @return A reference to this CharArrayWriter instance.
    auto append(const char* str) -> CharArrayWriter& override;

    /// @brief Appends an initializer list of characters to the buffer.
    /// @param chars The initializer list of characters to append.
    /// @return A reference to this CharArrayWriter instance.
    auto append(std::initializer_list<char> chars) -> CharArrayWriter& override;

    /// @brief Appends a sequence of characters to the buffer.
    /// @param chars Pointer to the character sequence.
    /// @param count Number of characters to append.
    /// @return A reference to this CharArrayWriter instance.
    auto append(const char* chars, size_t count) -> CharArrayWriter& override;

    /// @brief Appends a character multiple times to the buffer.
    /// @param c The character to append.
    /// @param count Number of times to append the character.
    /// @return A reference to this CharArrayWriter instance.
    auto append(char c, size_t count) -> CharArrayWriter& override;

    // Utility functions
    /// @brief Resets the buffer to empty.
    auto reset() -> void;

    /// @brief Returns a copy of the buffer's contents as a character array.
    /// @return A vector containing the buffer's contents.
    [[nodiscard]] auto toCharArray() const -> std::vector<char>;

    /// @brief Returns the current size of the buffer.
    /// @return The number of characters in the buffer.
    [[nodiscard]] auto size() const -> size_t;

    /// @brief Returns the buffer's contents as a string.
    /// @return A string representation of the buffer's contents.
    [[nodiscard]] auto toString() const -> std::string override;

    /// @brief Flushes the stream (no-op for this implementation).
    auto flush() -> void override;

    /// @brief Closes the writer (no-op for this implementation).
    auto close() -> void override;

    /// @brief Checks if the writer is closed.
    /// @return Always false for this implementation.
    [[nodiscard]] auto isClosed() const -> bool override;

private:
    std::vector<char> buf_{};
    size_t count_{0};

    /// @brief Ensures that the buffer has at least the specified capacity.
    /// @param minCapacity The minimum capacity required.
    auto ensureCapacity(size_t minCapacity) -> void;
};
}