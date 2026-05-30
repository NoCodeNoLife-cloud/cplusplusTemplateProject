/**
* @file Console.cc
 * @brief Console class implementation
 * @details This file contains the implementation of the Console class methods for Common library utilities.
 */

#include "src/filesystem/io/Console.hpp"

#include <fmt/format.h>

namespace common::filesystem
{
    void Console::flush()
    {
        std::cout.flush();
    }

    std::string Console::readLine()
    {
        std::string input;
        std::getline(std::cin, input);
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
