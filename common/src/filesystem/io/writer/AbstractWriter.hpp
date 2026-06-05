/**
 * @file AbstractWriter.hpp
 * @brief AbstractWriter class declaration
 * @details This header defines the AbstractWriter class that provides functionality for Common library utilities.
 */

#pragma once
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "interface/IAppendable.hpp"
#include "interface/ICloseable.hpp"
#include "interface/IFlushable.hpp"

namespace common::filesystem
{
    /// @brief Abstract base class for writers that provides basic functionality for writing characters and strings.
    class AbstractWriter : public interfaces::ICloseable,
                           public interfaces::IFlushable,
                           public interfaces::IAppendable<AbstractWriter>
    {
    public:
        AbstractWriter();

        ~AbstractWriter() override;

        /// @brief Appends a single character to the writer.
        /// @param c The character to append.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(char c) override;

        /// @brief Appends a string to the writer.
        /// @param csq The string to append.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(const std::string& csq) override;

        /// @brief Appends a subsequence of a string to the writer.
        /// @param csq The string from which a subsequence is to be appended.
        /// @param start The starting index of the subsequence to be appended.
        /// @param end The ending index of the subsequence to be appended.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(const std::string& csq, size_t start, size_t end) override;

        /// @brief Appends a string view to the writer.
        /// @param str The string view to append.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(std::string_view str) override;

        /// @brief Appends a C-string to the writer.
        /// @param str The C-string to append.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(const char* str) override;

        /// @brief Appends an initializer list of characters to the writer.
        /// @param chars The initializer list of characters to append.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(std::initializer_list<char> chars) override;

        /// @brief Appends a sequence of characters to the writer.
        /// @param chars Pointer to the character sequence.
        /// @param count Number of characters to append.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(const char* chars, size_t count) override;

        /// @brief Appends a character multiple times to the writer.
        /// @param c The character to append.
        /// @param count Number of times to append the character.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(char c, size_t count) override;

        /// @brief Appends a span of characters to the writer.
        /// @param chars The span of characters to append.
        /// @return A reference to this AbstractWriter instance.
        AbstractWriter& append(std::span<const char> chars) override;

        /// @brief Writes a single character to the writer.
        /// @param c The character to write.
        virtual void write(char c);

        /// @brief Writes a sequence of characters to the writer.
        /// @param cBuf The character sequence to write.
        virtual void write(const std::vector<char>& cBuf);

        /// @brief Writes a sequence of characters to the writer, starting from a specified offset.
        /// @param cBuf The character sequence to write.
        /// @param off The starting offset in the character sequence.
        /// @param len The number of characters to write.
        virtual void write(const std::vector<char>& cBuf, size_t off, size_t len) = 0;

        /// @brief Writes a string to the writer.
        /// @param str The string to write.
        virtual void write(const std::string& str);

        /// @brief Writes a substring to the writer, starting from a specified offset.
        /// @param str The string to write.
        /// @param off The starting offset in the string.
        /// @param len The number of characters to write.
        virtual void write(const std::string& str, size_t off, size_t len);

        /// @brief Converts the content of the writer to a string representation.
        /// @return A string representation of the writer's content.
        [[nodiscard]] virtual std::string toString() const = 0;

        /// @brief Flushes the writer.
        void flush() override = 0;
    };

    inline AbstractWriter::AbstractWriter() = default;

    inline AbstractWriter::~AbstractWriter() = default;

    inline AbstractWriter& AbstractWriter::append(const char c)
    {
        write(c);
        return *this;
    }

    inline AbstractWriter& AbstractWriter::append(const std::string& csq)
    {
        write(csq);
        return *this;
    }

    inline AbstractWriter& AbstractWriter::append(const std::string& csq, const size_t start, const size_t end)
    {
        if (start <= end && start <= csq.length())
        {
            const size_t safe_end = std::min(end, csq.length());
            write(csq, start, safe_end - start);
        }
        return *this;
    }

    inline AbstractWriter& AbstractWriter::append(std::string_view str)
    {
        if (!str.empty())
        {
            write(std::string(str));
        }
        return *this;
    }

    inline AbstractWriter& AbstractWriter::append(const char* str)
    {
        if (str)
        {
            write(std::string(str));
        }
        return *this;
    }

    inline AbstractWriter& AbstractWriter::append(const std::initializer_list<char> chars)
    {
        if (chars.size() > 0)
        {
            const std::vector<char> buf(chars);
            write(buf);
        }
        return *this;
    }

    inline AbstractWriter& AbstractWriter::append(const char* chars, const size_t count)
    {
        if (chars && count > 0)
        {
            const std::vector<char> buf(chars, chars + count);
            write(buf);
        }
        return *this;
    }

    inline AbstractWriter& AbstractWriter::append(const char c, const size_t count)
    {
        if (count > 0)
        {
            const std::vector<char> buf(count, c);
            write(buf);
        }
        return *this;
    }

    inline AbstractWriter& AbstractWriter::append(const std::span<const char> chars)
    {
        if (!chars.empty())
        {
            const std::vector<char> buf(chars.begin(), chars.end());
            write(buf);
        }
        return *this;
    }

    inline void AbstractWriter::write(const char c)
    {
        const std::vector<char> buf(1, c);
        write(buf, 0, 1);
    }

    inline void AbstractWriter::write(const std::vector<char>& cBuf)
    {
        if (!cBuf.empty())
        {
            write(cBuf, 0, cBuf.size());
        }
    }

    inline void AbstractWriter::write(const std::string& str)
    {
        if (!str.empty())
        {
            write(str, 0, str.size());
        }
    }

    inline void AbstractWriter::write(const std::string& str, const size_t off, const size_t len)
    {
        if (off < str.size() && len > 0)
        {
            const size_t end = std::min(off + len, str.size());
            const std::vector<char> buf(
                str.begin() + static_cast<std::string::difference_type>(off),
                str.begin() + static_cast<std::string::difference_type>(end));
            write(buf, 0, buf.size());
        }
    }
}
