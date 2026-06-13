/**
 * @file SHA256Strategy.cc
 * @brief SHA256Strategy implementation — EVP_Digest wrapper for SHA-256
 * @details Implements SHA-256 hash computation using OpenSSL's EVP_Digest
 *          interface.  Produces a 256-bit (32-byte) digest, hex-encoded.
 */

#include "SHA256Strategy.hpp"

#include <stdexcept>
#include <fmt/format.h>
#include <glog/logging.h>

namespace common::crypto::hash
{
    namespace
    {
        void evpDeleter(EVP_MD_CTX* ctx)
        {
            EVP_MD_CTX_free(ctx);
        }
    }

    SHA256Strategy::SHA256Strategy() : ctx_(EVP_MD_CTX_new(), evpDeleter)
    {
        if (!ctx_)
        {
            DLOG(WARNING) << "Failed to allocate EVP_MD_CTX for SHA-256";
            throw std::runtime_error("Failed to allocate EVP_MD_CTX");
        }
        if (EVP_DigestInit_ex(ctx_.get(), EVP_sha256(), nullptr) != 1)
        {
            DLOG(WARNING) << "Failed to initialize SHA-256 context";
            throw std::runtime_error("Failed to initialize SHA-256 context");
        }
    }

    SHA256Strategy::SHA256Strategy(SHA256Strategy&& other) noexcept : ctx_(std::move(other.ctx_)), finalized_(other.finalized_)
    {
        other.finalized_ = true; // Prevent other from being used after move
    }

    SHA256Strategy& SHA256Strategy::operator=(SHA256Strategy&& other) noexcept
    {
        if (this != &other)
        {
            ctx_ = std::move(other.ctx_);
            finalized_ = other.finalized_;
            other.finalized_ = true; // Prevent other from being used after move
        }
        return *this;
    }

    SHA256Strategy::~SHA256Strategy() = default;

    size_t SHA256Strategy::getDigestSize() const
    {
        return DIGEST_SIZE;
    }

    size_t SHA256Strategy::getHexDigestSize() const
    {
        return HEX_DIGEST_SIZE;
    }

    bool SHA256Strategy::update(const void* data, size_t length)
    {
        if (finalized_)
        {
            return false;
        }
        return EVP_DigestUpdate(ctx_.get(), data, length) == 1;
    }

    std::optional<std::vector<uint8_t>> SHA256Strategy::finalize()
    {
        if (finalized_)
        {
            return std::nullopt;
        }

        std::vector<uint8_t> digest(DIGEST_SIZE);
        unsigned int length = 0;

        if (EVP_DigestFinal_ex(ctx_.get(), digest.data(), &length) != 1)
        {
            return std::nullopt;
        }

        if (length != DIGEST_SIZE)
        {
            return std::nullopt;
        }

        finalized_ = true;
        return digest;
    }

    bool SHA256Strategy::reset()
    {
        if (!ctx_)
        {
            return false;
        }
        finalized_ = false;
        return EVP_DigestInit_ex(ctx_.get(), EVP_sha256(), nullptr) == 1;
    }

}
