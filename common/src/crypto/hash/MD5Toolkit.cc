#include "MD5Toolkit.hpp"

#include <sstream>
#include <iomanip>

namespace common::crypto::hash {
    /// @brief Append string data to hash computation
    /// @param data The string data to hash
    auto MD5Toolkit::update(const std::string &data) -> void {
        if (!data.empty()) {
            md5_.process_bytes(data.data(), data.size());
        }
    }

    /// @brief Append binary data to hash computation
    /// @param data Pointer to the binary data
    /// @param length Length of the binary data in bytes
    auto MD5Toolkit::update(const void *data, const size_t length) -> void {
        if (data && length > 0) {
            md5_.process_bytes(data, length);
        }
    }

    /// @brief Complete computation and return MD5 hex string
    /// @return MD5 hash as a 32-character lowercase hexadecimal string
    auto MD5Toolkit::finalize() -> std::string {
        boost::uuids::detail::md5::digest_type digest;
        md5_.get_digest(digest);

        // Convert 16 bytes (4 uint32_t) to hex string
        const auto *bytes = reinterpret_cast<const unsigned char *>(digest);
        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        for (int i = 0; i < 16; ++i) {
            oss << std::setw(2) << static_cast<int>(bytes[i]);
        }

        return oss.str();
    }

    /// @brief Reset internal state for reuse
    auto MD5Toolkit::reset() -> void {
        md5_ = boost::uuids::detail::md5();
    }

    /// @brief One-shot hash for string
    /// @param data The string data to hash
    /// @return MD5 hash as a 32-character lowercase hexadecimal string
    [[nodiscard]] auto MD5Toolkit::hash(const std::string &data) -> std::string {
        MD5Toolkit toolkit;
        toolkit.update(data);
        return toolkit.finalize();
    }

    /// @brief One-shot hash for binary data
    /// @param data Pointer to the binary data
    /// @param length Length of the binary data in bytes
    /// @return MD5 hash as a 32-character lowercase hexadecimal string
    [[nodiscard]] auto MD5Toolkit::hash(const void *data, const size_t length) -> std::string {
        MD5Toolkit toolkit;
        toolkit.update(data, length);
        return toolkit.finalize();
    }
}
