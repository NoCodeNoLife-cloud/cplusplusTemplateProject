#include "src/filesystem/io/Console.hpp"

#include <fmt/format.h>

namespace common::filesystem {
    auto Console::flush() -> void {
        std::cout.flush();
    }

    auto Console::readLine() -> std::string {
        std::string input;
        std::getline(std::cin, input);
        return input;
    }

    auto Console::writer() -> std::ostream & {
        return std::cout;
    }

    auto Console::reader() -> std::istream & {
        return std::cin;
    }
}
