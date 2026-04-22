#include "AuthenticationException.hpp"
#include <glog/logging.h>
#include <fmt/format.h>
#include <string_view>

namespace common::exception {
    AuthenticationException::AuthenticationException(const std::string &message) : std::runtime_error(message) {
        DLOG(ERROR) << fmt::format("AuthenticationException thrown - message: {}", message);
    }

    AuthenticationException::AuthenticationException(const std::string_view message) : std::runtime_error(std::string(message)) {
        DLOG(ERROR) << fmt::format("AuthenticationException thrown (string_view) - message: {}", message);
    }

    AuthenticationException::AuthenticationException(AuthenticationException &&other) noexcept : std::runtime_error(other.what()) {
    }

    AuthenticationException::AuthenticationException(const AuthenticationException &other) : std::runtime_error(other.what()) {
    }

    AuthenticationException::~AuthenticationException() noexcept = default;
} // common
