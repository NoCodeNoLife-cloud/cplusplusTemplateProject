/**
 * @file Console.cc
 * @brief Console implementation — ANSI escape codes, progress bar rendering
 * @details Implements console I/O methods: ANSI escape sequences for colour,
 *          carriage-return-based progress bar, and stdin/stdout interaction.
 */

#include "io/Console.hpp"

namespace common::io
{
    std::mutex Console::s_mutex_;

    void Console::flush()
    {
        std::lock_guard<std::mutex> lock(s_mutex_);
        std::cout.flush();
    }

    bool Console::isFlushNeeded() const
    {
        return true;
    }

    std::string Console::readLine()
    {
        std::lock_guard<std::mutex> lock(s_mutex_);
        std::string input;
        std::getline(std::cin, input);
        if (std::cin.fail() && !std::cin.eof())
        {
            std::cin.clear();
        }
        return input;
    }

    std::ostream& Console::writer()
    {
        return std::cout;
    }

    std::istream& Console::reader()
    {
        return std::cin;
    }
}
