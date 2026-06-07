/**
 * @file Console.cc
 * @brief Console class implementation
 * @details This file contains the implementation of the Console class methods for Common library utilities.
 */

#include "filesystem/io/Console.hpp"

namespace common::filesystem::io
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
