#include "src/filesystem/io/Console.hpp"

#include <glog/logging.h>
#include <fmt/format.h>

namespace common::filesystem {
    auto Console::flush() -> void {
        DLOG(INFO) << "Console flush - flushing stdout";
        std::cout.flush();
    }

    auto Console::readLine() -> std::string {
        DLOG(INFO) << "Console readLine - reading input from stdin";
        std::string input;
        std::getline(std::cin, input);
        DLOG(INFO) << fmt::format("Console readLine - read {} characters", input.length());
        return input;
    }

    auto Console::writer() -> std::ostream & {
        return std::cout;
    }

    auto Console::reader() -> std::istream & {
        return std::cin;
    }
}
