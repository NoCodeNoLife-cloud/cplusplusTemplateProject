#include <cppforge/toolkit/StringToolkit.hpp>
#include <cppforge/toolkit/IntegerToolkit.hpp>
#include <cppforge/time/Date.hpp>
#include <cppforge/gen/UuidGenerator.hpp>
#include <iostream>

int main() {
    std::cout << "=== cppforge Sample Project ===" << std::endl;

    // Test StringToolkit
    auto parts = cppforge::toolkit::StringToolkit::split("hello world", ' ');
    std::cout << "StringToolkit::split: ";
    for (const auto& p : parts)
        std::cout << "[" << p << "] ";
    std::cout << std::endl;

    // Test IntegerToolkit
    int val = cppforge::toolkit::IntegerToolkit::romanToInt("XLII");
    std::cout << "IntegerToolkit::romanToInt: " << val << std::endl;

    // Test Date
    cppforge::time::Date now;
    std::cout << "Date::toString: " << now.toString() << std::endl;

    // Test UuidGenerator
    auto uuid = cppforge::gen::UuidGenerator::GenerateRandomUuid();
    std::cout << "UuidGenerator::GenerateRandomUuid: " << uuid << std::endl;

    std::cout << "=== All tests passed ===" << std::endl;
    return 0;
}
