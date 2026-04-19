#pragma once
#include <boost/uuid/detail/md5.hpp>
#include <string>

namespace common::toolkit::hash {
    class MD5Toolkit {
    public:
        MD5Toolkit() = default;

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
        static auto hash(const std::string &data) -> std::string;

        /// @brief One-shot hash for binary data
        /// @param data Pointer to the binary data
        /// @param length Length of the binary data in bytes
        /// @return MD5 hash as a 32-character lowercase hexadecimal string
        static auto hash(const void *data, size_t length) -> std::string;

    private:
        boost::uuids::detail::md5 md5;
    };
}
