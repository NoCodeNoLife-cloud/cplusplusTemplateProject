/**
 * @file JwtToken.hpp
 * @brief Simplified JWT token encoder and decoder using HMAC-SHA256
 * @details Provides encode/decode for JWT tokens with a string-map payload.
 *          Uses OpenSSL HMAC-SHA256 for signing. Format: base64(header).base64(payload).base64(signature).
 */

#pragma once
#include <chrono>
#include <map>
#include <optional>
#include <string>

namespace cppforge::starter::security
{
    /// @brief JWT token encoder and decoder
    /// @details Implements a simplified JWT with HMAC-SHA256 signing via OpenSSL.
    class JwtToken
    {
    public:
        /// @brief Construct a JwtToken handler with the given secret
        /// @param secret The HMAC signing secret
        explicit JwtToken(std::string secret);

        /// @brief Encode a payload into a JWT token string
        /// @param payload Key-value pairs to include in the token
        /// @return The encoded JWT string (header.payload.signature)
        [[nodiscard]] std::string encode(const std::map<std::string, std::string>& payload) const;

        /// @brief Decode and verify a JWT token
        /// @param token The JWT string to decode
        /// @return The decoded payload if valid, std::nullopt if invalid or expired
        [[nodiscard]] std::optional<std::map<std::string, std::string>> decode(const std::string& token) const;

    private:
        const std::string secret_;

        /// @brief Base64url encode a string (RFC 4648)
        /// @param input The input bytes
        /// @return Base64url-encoded string
        [[nodiscard]] static std::string base64UrlEncode(const std::string& input);

        /// @brief Base64url decode a string (RFC 4648)
        /// @param input The base64url-encoded string
        /// @return Decoded string
        [[nodiscard]] static std::string base64UrlDecode(const std::string& input);

        /// @brief Compute HMAC-SHA256 signature
        /// @param data The data to sign
        /// @param key The signing key
        /// @return Raw signature bytes
        [[nodiscard]] static std::string hmacSha256(const std::string& data, const std::string& key);
    };
}