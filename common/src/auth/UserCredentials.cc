#include "UserCredentials.hpp"


#include <fmt/format.h>
#include <utility>
#include <chrono>

namespace common::auth {
    UserCredentials::UserCredentials(std::string username, std::string hashed_password, std::string salt) noexcept : username_(std::move(username)), hashed_password_(std::move(hashed_password)), salt_(std::move(salt)), failed_attempts_(0), last_failed_attempt_(std::chrono::system_clock::time_point::min()) {
    }

    auto UserCredentials::get_username() const noexcept -> const std::string & {
        return username_;
    }

    auto UserCredentials::get_hashed_password() const noexcept -> const std::string & {
        return hashed_password_;
    }

    auto UserCredentials::get_salt() const noexcept -> const std::string & {
        return salt_;
    }

    auto UserCredentials::get_failed_attempts() const noexcept -> size_t {
        return failed_attempts_;
    }

    auto UserCredentials::increment_failed_attempts() noexcept -> void {
        failed_attempts_++;
        last_failed_attempt_ = std::chrono::system_clock::now();
    }

    auto UserCredentials::reset_failed_attempts() noexcept -> void {
        failed_attempts_ = 0;
        last_failed_attempt_ = std::chrono::system_clock::time_point::min();
    }

    auto UserCredentials::is_locked() const noexcept -> bool {
        return is_locked(DEFAULT_LOCKOUT_DURATION, DEFAULT_MAX_ATTEMPTS);
    }

    auto UserCredentials::is_locked(const std::chrono::minutes lockout_duration, const size_t max_attempts) const noexcept -> bool {
        const auto now = std::chrono::system_clock::now();
        const auto time_since_last_fail = now - last_failed_attempt_;
        const auto minutes_since_last_fail = std::chrono::duration_cast<std::chrono::minutes>(time_since_last_fail);

        // Account locked if max attempts reached and lockout period hasn't expired
        return failed_attempts_ >= max_attempts && minutes_since_last_fail < lockout_duration;
    }
} // common
