#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <stdexcept>

namespace common::service::interfaces {
    /// @brief Interface for application execution functionality.
    /// This interface defines the contract for classes that execute applications
    /// with command-line arguments. Implementations should handle the application
    /// lifecycle and return appropriate success/failure status.
    class IApplicationExecutor {
    public:
        virtual ~IApplicationExecutor() = default;

        /// @brief Executes the application with the given command-line arguments.
        /// @param argc The number of command-line arguments.
        /// @param argv An array of pointers to null-terminated strings representing the command-line arguments.
        /// @return true if the execution was successful, false otherwise.
        /// @throws std::runtime_error if execution fails with details about the failure
        [[nodiscard]] virtual auto execute(int32_t argc, char *argv[]) noexcept -> bool = 0;

        /// @brief Executes the application with vector of string arguments (modern alternative).
        /// @param args Vector of string arguments representing the command-line arguments.
        /// @return true if the execution was successful, false otherwise.
        /// @throws std::runtime_error if execution fails with details about the failure
        [[nodiscard]] virtual auto execute(const std::vector<std::string> &args) noexcept -> bool = 0;
    };
}
