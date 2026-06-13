/**
 * @file Scanner.hpp
 * @brief Token-based input scanner for integers, doubles, lines, and delimited tokens
 * @details A simple lexical scanner wrapping a std::istream.  Provides
 *          nextInt(), nextDouble(), nextLine(), and configurable delimiter-based
 *          tokenisation (nextTokens).  Designed for structured input parsing
 *          without the overhead of a full regex-based scanner.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  The underlying std::istream must not be
 * shared with other readers during scanning.
 *
 * @par Usage Example
 * @code
 * std::istringstream input("42 3.14 hello world");
 * Scanner scanner(input);
 * int i = scanner.nextInt();
 * double d = scanner.nextDouble();
 * auto tokens = scanner.nextTokens(' ');
 * @endcode
 */

#pragma once
#include <string>
#include <vector>

namespace common::io::reader
{
    /// @brief A utility class for scanning and parsing input streams.
    /// The Scanner class provides methods for reading various data types from an input stream,
    /// such as integers, doubles, strings, and lines. It also supports reading tokens separated
    /// by delimiters. This class is designed to simplify input parsing operations.
    class Scanner
    {
    public:
        explicit Scanner(std::istream& input_stream);

        /// @brief Reads the next integer from the input stream.
        /// This method parses the next token from the input stream and converts it to a 32-bit signed integer.
        /// It assumes that the next token in the stream represents a valid integer.
        /// @return The parsed integer value.
        /// @throws std::runtime_error if no more integers are available or parsing fails.
        /// @throws std::invalid_argument if the token cannot be converted to an integer.
        /// @throws std::out_of_range if the token represents a number out of the range of int32_t.
        [[nodiscard]] int32_t nextInt();

        /// @brief Reads the next double from the input stream.
        /// This method parses the next token from the input stream and converts it to a double precision floating point
        /// number. It assumes that the next token in the stream represents a valid double.
        /// @return The parsed double value.
        /// @throws std::runtime_error if no more doubles are available or parsing fails.
        /// @throws std::invalid_argument if the token cannot be converted to a double.
        /// @throws std::out_of_range if the token represents a number out of the range of double.
        [[nodiscard]] double nextDouble();

        /// @brief Reads the next line from the input stream.
        /// This method reads characters from the input stream until it encounters a newline character or the end of the
        /// stream.
        /// @return The next line as a string, excluding the newline character.
        [[nodiscard]] std::string nextLine();

        /// @brief Reads the next token from the input stream.
        /// This method reads characters from the input stream until it encounters a whitespace character or the end of the
        /// stream.
        /// @param token A reference to a string where the next token will be stored.
        /// @return `true` if a token was successfully read, `false` if the end of the stream was reached.
        bool getNextToken(std::string& token);

        /// @brief Reads all tokens from the input stream until the end of the stream is reached.
        /// Tokens are separated by the specified delimiter character. By default, the delimiter is a space (' ').
        /// This method returns a vector containing all the tokens read from the stream.
        /// @param delimiter The character used to separate tokens in the input stream. Defaults to space (' ').
        /// @return A vector of strings, where each string is a token from the input stream.
        [[nodiscard]] std::vector<std::string> nextTokens(char delimiter = ' ');

    private:
        std::istream& input_;
    };
}