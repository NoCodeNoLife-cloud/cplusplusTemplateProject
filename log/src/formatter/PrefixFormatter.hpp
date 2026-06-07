/**
* @file PrefixFormatter.hpp
 * @brief Custom log prefix formatter for glog
 * @details This header defines the PrefixFormatter class that provides
 *          a customizable prefix format for glog logging messages, including
 *          severity level, timestamp, thread ID, and source location information.
 */

#pragma once
#include <iosfwd>
#include <glog/logging.h>

namespace glog::formatter
{
    /// @brief Custom formatter for glog prefix
    /// Provides a customizable prefix format for glog logging messages
    class PrefixFormatter final
    {
    public:
        /// @brief Default constructor
        constexpr PrefixFormatter() noexcept = default;

        /// @brief Destructor
        ~PrefixFormatter() noexcept = default;

        /// @brief Copy constructor
        constexpr PrefixFormatter(const PrefixFormatter&) noexcept = default;

        /// @brief Copy assignment operator
        constexpr PrefixFormatter& operator=(const PrefixFormatter&) noexcept = default;

        /// @brief Move constructor
        constexpr PrefixFormatter(PrefixFormatter&&) noexcept = default;

        /// @brief Move assignment operator
        constexpr PrefixFormatter& operator=(PrefixFormatter&&) noexcept = default;

        /// @brief Formats log message prefix according to custom specification
        /// @param s Output stream to write formatted prefix to
        /// @param m Log message containing metadata for the prefix
        /// @param data User data pointer (unused)
        static auto MyPrefixFormatter(std::ostream& s, const google::LogMessage& m, [[maybe_unused]] void* data) noexcept -> void;

    private:
        /// @brief Date format helper constant for year offset
        static constexpr int kYearOffset_ = 1900;

        /// @brief Date format helper constant for month offset
        static constexpr int kMonthOffset_ = 1;
    };
}