/**
 * @file PrefixFormatter.cc
 * @brief Implementation of custom glog prefix formatter
 * @details This file contains the implementation of the MyPrefixFormatter method
 *          that formats log messages with severity, timestamp, thread ID, and source location.
 */

#include <cppforge/glog/formatter/PrefixFormatter.hpp>

#include <iomanip>

namespace cppforge::glog::formatter
{
    auto PrefixFormatter::MyPrefixFormatter(std::ostream& s, const google::LogMessage& m, [[maybe_unused]] void* data) noexcept -> void
    {
        const char old_fill = s.fill('0');
        s << '[' << google::GetLogSeverityName(m.severity()) << "] "
          << "[" << std::setw(4) << (kYearOffset_ + m.time().year())
          << std::setw(2) << (kMonthOffset_ + m.time().month())
          << std::setw(2) << m.time().day() << ' '
          << std::setw(2) << m.time().hour() << ':'
          << std::setw(2) << m.time().min() << ':'
          << std::setw(2) << m.time().sec() << "."
          << std::setw(6) << m.time().usec() << "] "
          << "[" << std::setfill(' ') << std::setw(5) << m.thread_id() << "] "
          << "[" << m.basename() << ':' << m.line() << "] ";
        s.fill(old_fill);
    }
}
