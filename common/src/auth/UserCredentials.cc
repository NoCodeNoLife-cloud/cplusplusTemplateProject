/**
 * @file UserCredentials.cc
 * @brief UserCredentials class implementation
 * @details This file contains the implementation of the UserCredentials class methods for Authentication and authorization utilities.
 */

#include "UserCredentials.hpp"

#include <fmt/format.h>
#include <utility>
#include <chrono>

#include <glog/logging.h>

namespace common::auth
{
    UserCredentials::UserCredentials(std::string username, std::string hashed_password, std::string salt)
        : username_(std::move(username)),
          hashed_password_(std::move(hashed_password)),
          salt_(std::move(salt)),
          failed_attempts_(0),
          last_failed_attempt_(std::chrono::system_clock::time_point::min())
    {
    }

    const std::string& UserCredentials::get_username() const
    {
        return username_;
    }

    const std::string& UserCredentials::get_hashed_password() const
    {
        return hashed_password_;
    }

    const std::string& UserCredentials::get_salt() const
    {
        return salt_;
    }

    size_t UserCredentials::get_failed_attempts() const
    {
        return failed_attempts_;
    }

    void UserCredentials::increment_failed_attempts()
    {
        failed_attempts_++;
        last_failed_attempt_ = std::chrono::system_clock::now();
        // Only log when reaching critical thresholds to avoid spam
        if (failed_attempts_ == 3 || failed_attempts_ == 5)
        {
            DLOG(WARNING) << "Failed attempt warning for user: " << username_
                << " (total: " << failed_attempts_ << ")";
        }
    }

    void UserCredentials::reset_failed_attempts()
    {
        // Only log if there were previous failures to report
        if (failed_attempts_ > 0)
        {
            DLOG(INFO) << "Resetting failed attempts for user: " << username_
                << " (previous count: " << failed_attempts_ << ")";
        }
        failed_attempts_ = 0;
        last_failed_attempt_ = std::chrono::system_clock::time_point::min();
    }

    bool UserCredentials::is_locked() const
    {
        return is_locked(DEFAULT_LOCKOUT_DURATION, DEFAULT_MAX_ATTEMPTS);
    }

    bool UserCredentials::is_locked(const std::chrono::minutes lockout_duration, const size_t max_attempts) const
    {
        const auto now = std::chrono::system_clock::now();
        const auto time_since_last_fail = now - last_failed_attempt_;
        const auto minutes_since_last_fail = std::chrono::duration_cast<std::chrono::minutes>(time_since_last_fail);

        // Account locked if max attempts reached and lockout period hasn't expired
        const bool locked = failed_attempts_ >= max_attempts && minutes_since_last_fail < lockout_duration;
        if (locked)
        {
            DLOG(WARNING) << "Account locked for user: " << username_
                << " (attempts: " << failed_attempts_ << ", lockout duration: " << lockout_duration.count() << "min)";
        }
        return locked;
    }
}