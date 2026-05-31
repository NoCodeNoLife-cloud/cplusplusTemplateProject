/**
 * @file Console.hpp
 * @brief Console class declaration
 * @details This header defines the Console class that provides functionality for Common library utilities.
 */

#pragma once
#include <format>
#include <iostream>
#include <string>

#include "interface/IFlushable.hpp"

namespace common::filesystem
{
    /// @brief Provides console input/output operations with formatting capabilities.
    /// This class implements the IFlushable interface and offers methods for formatted
    /// output, input reading, and stream management.
    class Console final : public interfaces::IFlushable
    {
    public:
        /// @brief Formats and prints a string to the console.
        /// @tparam Args Variadic template arguments.
        /// @param fmt The format string.
        /// @param args Arguments to be formatted.
        template <typename... Args>
        void format(const std::string& fmt, Args&&... args) const;

        /// @brief Prints a formatted string to the console.
        /// @tparam Args Variadic template arguments.
        /// @param fmt The format string.
        /// @param args Arguments to be formatted.
        template <typename... Args>
        void printf(const std::string& fmt, Args&&... args) const;

        /// @brief Flushes the console output.
        void flush() override;

        /// @brief Reads a line from the console.
        /// @return The read line as a string.
        static std::string readLine();

        /// @brief Reads a line from the console with a prompt.
        /// @tparam Args Variadic template arguments for the prompt formatting.
        /// @param fmt The format string for the prompt.
        /// @param args Arguments to be formatted in the prompt.
        /// @return The read line as a string.
        template <typename... Args>
        static std::string readLine(const std::string& fmt, Args&&... args);

        /// @brief Gets the writer stream for the console.
        /// @return Reference to the output stream.
        static std::ostream& writer();

        /// @brief Gets the reader stream for the console.
        /// @return Reference to the input stream.
        static std::istream& reader();
    };

    template <typename... Args>
    void Console::format(const std::string& fmt, Args&&... args) const
    {
        std::cout << std::vformat(fmt, std::make_format_args(args...));
    }

    template <typename... Args>
    void Console::printf(const std::string& fmt, Args&&... args) const
    {
        format(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    std::string Console::readLine(const std::string& fmt, Args&&... args)
    {
        Console{}.format(fmt, std::forward<Args>(args)...);
        return readLine();
    }
}