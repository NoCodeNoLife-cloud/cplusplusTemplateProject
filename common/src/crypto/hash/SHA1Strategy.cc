/**
 * @file SHA1Strategy.cc
 * @brief SHA1Strategy implementation â€?EVP_Digest wrapper for SHA-1
 * @details Implements SHA-1 hash computation using OpenSSL's EVP_Digest
 *          interface.  Produces a 160-bit (20-byte) digest, hex-encoded.
 */

#include <cppforge/crypto/hash/SHA1Strategy.hpp>

#include <stdexcept>
#include <fmt/format.h>
#include <glog/logging.h>

namespace cppforge::crypto::hash
{
    namespace
    {
        void evpDeleter(EVP_MD_CTX* ctx)
        {
            if (ctx != nullptr)
            {
                EVP_MD_CTX_free(ctx);
            }
        }
    }

    SHA1Strategy::SHA1Strategy() : ctx_(EVP_MD_CTX_new(), evpDeleter)
    {
        if (!ctx_)
        {
            DLOG(WARNING) << "Failed to allocate EVP_MD_CTX for SHA-1";
            throw std::runtime_error("Failed to allocate EVP_MD_CTX");
        }
        if (EVP_DigestInit_ex(ctx_.get(), EVP_sha1(), nullptr) != 1)
        {
            DLOG(WARNING) << "Failed to initialize SHA-1 context";
            throw std::runtime_error("Failed to initialize SHA-1 context");
        }
    }

    SHA1Strategy::SHA1Strategy(SHA1Strategy&& other) noexcept : ctx_(std::move(other.ctx_)), finalized_(other.finalized_)
    {
        other.finalized_ = true; // Prevent other from being used after move
    }

    SHA1Strategy& SHA1Strategy::operator=(SHA1Strategy&& other) noexcept
    {
        if (this != &other)
        {
            ctx_ = std::move(other.ctx_);
            finalized_ = other.finalized_;
            other.finalized_ = true; // Prevent other from being used after move
        }
        return *this;
    }

    SHA1Strategy::~SHA1Strategy() = default;

    size_t SHA1Strategy::getDigestSize() const
    {
        return DIGEST_SIZE;
    }

    size_t SHA1Strategy::getHexDigestSize() const
    {
        return HEX_DIGEST_SIZE;
    }

    bool SHA1Strategy::update(const void* data, size_t length)
    {
        if (finalized_)
        {
            return false;
        }
        return EVP_DigestUpdate(ctx_.get(), data, length) == 1;
    }

    std::optional<std::vector<uint8_t>> SHA1Strategy::finalize()
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

    bool SHA1Strategy::reset()
    {
        if (!ctx_)
        {
            return false;
        }
        finalized_ = false;
        return EVP_DigestInit_ex(ctx_.get(), EVP_sha1(), nullptr) == 1;
    }

}
