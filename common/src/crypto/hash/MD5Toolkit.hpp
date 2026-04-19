#pragma once
#include <boost/uuid/detail/md5.hpp>
#include <string>

namespace common::crypto::hash {
    /// @brief MD5 hash toolkit providing hashing functionality
    /// @details This class provides both one-shot and incremental MD5 hashing capabilities.
    /// It wraps the Boost MD5 implementation and provides a convenient interface for
    /// computing MD5 hashes of strings and binary data.
    class MD5Toolkit final {
    public:
        /// @brief Default constructor
        constexpr MD5Toolkit() noexcept = default;

        /// @brief Destructor
        ~MD5Toolkit() noexcept = default;

        /// @brief Copy constructor
        constexpr MD5Toolkit(const MD5Toolkit &) noexcept = default;

        /// @brief Copy assignment operator
        constexpr auto operator=(const MD5Toolkit &) noexcept -> MD5Toolkit & = default;

        /// @brief Move constructor
        constexpr MD5Toolkit(MD5Toolkit &&) noexcept = default;

        /// @brief Move assignment operator
        constexpr auto operator=(MD5Toolkit &&) noexcept -> MD5Toolkit & = default;

        /// @brief Append string data to hash computation
        /// @param data The string data to hash
        auto update(const std::string &data) -> void;

        /// @brief Append binary data to hash computation
        /// @param data Pointer to the binary data
        /// @param length Length of the binary data in bytes
        auto update(const void *data, size_t length) -> void;

        /// @brief Complete computation and return MD5 hex string
        /// @return MD5 hash as a 32-character lowercase hexadecimal string
        auto finalize() -> std::string;

        /// @brief Reset internal state for reuse
        auto reset() -> void;

        /// @brief One-shot hash for string
        /// @param data The string data to hash
        /// @return MD5 hash as a 32-character lowercase hexadecimal string
        [[nodiscard]] static auto hash(const std::string &data) -> std::string;

        /// @brief One-shot hash for binary data
        /// @param data Pointer to the binary data
        /// @param length Length of the binary data in bytes
        /// @return MD5 hash as a 32-character lowercase hexadecimal string
        [[nodiscard]] static auto hash(const void *data, size_t length) -> std::string;

    private:
        boost::uuids::detail::md5 md5_;
    };
}
