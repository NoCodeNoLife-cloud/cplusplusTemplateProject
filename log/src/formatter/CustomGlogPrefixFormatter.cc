/**
 * @file CustomGlogPrefixFormatter.cc
 * @brief Implementation of custom glog prefix formatter
 * @details This file contains the implementation of the MyPrefixFormatter method
 *          that formats log messages with severity, timestamp, thread ID, and source location.
 */

#include "CustomGlogPrefixFormatter.hpp"

#include <iomanip>

namespace glog::formatter
{
    /// @brief Formats log message prefix according to custom specification
    /// @param s Output stream to write formatted prefix to
    /// @param m Log message containing metadata for the prefix
    /// @param data User data pointer (unused)
    auto CustomGlogPrefixFormatter::MyPrefixFormatter(std::ostream& s, const google::LogMessage& m, [[maybe_unused]] void* data) noexcept -> void
    {
        // Use the original stream-based approach to avoid fmt compatibility issues
        s << '[' << google::GetLogSeverityName(m.severity()) << "] "
            << "[" << std::setw(4) << (kYearOffset_ + m.time().year())
            << std::setw(2) << (kMonthOffset_ + m.time().month())
            << std::setw(2) << m.time().day() << ' '
            << std::setw(2) << m.time().hour() << ':'
            << std::setw(2) << m.time().min() << ':'
            << std::setw(2) << m.time().sec() << "."
            << std::setw(6) << m.time().usec() << "] "
            << "[" << std::setfill(' ') << std::setw(5) << m.thread_id() << std::setfill('0') << "] "
            << "[" << m.basename() << ':' << m.line() << "] ";
    }
}
