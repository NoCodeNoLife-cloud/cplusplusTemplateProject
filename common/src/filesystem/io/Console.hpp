#pragma once
#include <format>
#include <iostream>
#include <string>

#include "src/interface/IFlushable.hpp"

namespace common::filesystem {
    /// @brief Provides console input/output operations with formatting capabilities.
    /// This class implements the IFlushable interface and offers methods for formatted
    /// output, input reading, and stream management.
    class Console final : public interfaces::IFlushable {
    public:
        /// @brief Formats and prints a string to the console.
        /// @tparam Args Variadic template arguments.
        /// @param fmt The format string.
        /// @param args Arguments to be formatted.
        template<typename... Args>
        auto format(const std::string &fmt, Args &&... args) const -> void;

        /// @brief Prints a formatted string to the console.
        /// @tparam Args Variadic template arguments.
        /// @param fmt The format string.
        /// @param args Arguments to be formatted.
        template<typename... Args>
        auto printf(const std::string &fmt, Args &&... args) const -> void;

        /// @brief Flushes the console output.
        auto flush() -> void override;

        /// @brief Reads a line from the console.
        /// @return The read line as a string.
        static auto readLine() -> std::string;

        /// @brief Reads a line from the console with a prompt.
        /// @tparam Args Variadic template arguments for the prompt formatting.
        /// @param fmt The format string for the prompt.
        /// @param args Arguments to be formatted in the prompt.
        /// @return The read line as a string.
        template<typename... Args>
        static auto readLine(const std::string &fmt, Args &&... args) -> std::string;

        /// @brief Gets the writer stream for the console.
        /// @return Reference to the output stream.
        static auto writer() -> std::ostream &;

        /// @brief Gets the reader stream for the console.
        /// @return Reference to the input stream.
        static auto reader() -> std::istream &;
    };

    template<typename... Args>
    auto Console::format(const std::string &fmt, Args &&... args) const -> void {
        std::cout << std::vformat(fmt, std::make_format_args(args...));
    }

    template<typename... Args>
    auto Console::printf(const std::string &fmt, Args &&... args) const -> void {
        format(fmt, std::forward<Args>(args)...);
    }

    template<typename... Args>
    auto Console::readLine(const std::string &fmt, Args &&... args) -> std::string {
        Console{}.format(fmt, std::forward<Args>(args)...);
        return readLine();
    }
}
