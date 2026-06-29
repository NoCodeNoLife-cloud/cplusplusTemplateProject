/**
 * @file Console.hpp
 * @brief Console I/O utilities â€?colour output, progress bar, prompts
 * @description Provides static methods for interactive console I/O: coloured
 *          text output (ANSI escape codes), progress bar display, user prompt
 *          with confirmation, and formatted table output.  Works on both
 *          POSIX terminals and Windows console.
 *
 * @par Thread Safety
 * Static methods that write to stdout/stderr are not thread-safe; external
 * synchronisation is needed for concurrent output.
 */

#pragma once
#include <format>
#include <iostream>
#include <mutex>
#include <string>
#include <string_view>

#include "interface/io/IFlushable.hpp"

namespace cppforge::io
{
    /// @brief Provides console input/output operations with formatting capabilities.
    /// This class implements the IFlushable interface and offers methods for formatted
    /// output, input reading, and stream management.
    class Console final : public interface::io::IFlushable
    {
    public:
        /// @brief Formats and prints a string to the console.
        /// @tparam Args Variadic template arguments.
        /// @param fmt The format string.
        /// @param args Arguments to be formatted.
        template <typename... Args>
        static void format(std::string_view fmt, Args&&... args);

        /// @brief Flushes the console output.
        void flush() override;

        /// @brief Checks if the console output needs to be flushed.
        /// @return true always.
        [[nodiscard]] bool isFlushNeeded() const override;

        /// @brief Reads a line from the console.
        /// @return The read line as a string.
        [[nodiscard]] static std::string readLine();

        /// @brief Reads a line from the console with a prompt.
        /// @tparam Args Variadic template arguments for the prompt formatting.
        /// @param fmt The format string for the prompt.
        /// @param args Arguments to be formatted in the prompt.
        /// @return The read line as a string.
        template <typename... Args>
        [[nodiscard]] static std::string readLine(std::string_view fmt, Args&&... args);

        /// @brief Gets the writer stream for the console.
        /// @return Reference to the output stream.
        [[nodiscard]] static std::ostream& writer();

        /// @brief Gets the reader stream for the console.
        /// @return Reference to the input stream.
        [[nodiscard]] static std::istream& reader();

    private:
        static std::mutex s_mutex_;
    };

    template <typename... Args>
    void Console::format(std::string_view fmt, Args&&... args)
    {
        std::lock_guard<std::mutex> lock(s_mutex_);
        std::cout << std::vformat(fmt, std::make_format_args(args...));
    }

    template <typename... Args>
    std::string Console::readLine(std::string_view fmt, Args&&... args)
    {
        std::lock_guard<std::mutex> lock(s_mutex_);
        std::cout << std::vformat(fmt, std::make_format_args(args...));
        std::string input;
        std::getline(std::cin, input);
        if (std::cin.fail() && !std::cin.eof())
        {
            std::cin.clear();
        }
        return input;
    }
}
