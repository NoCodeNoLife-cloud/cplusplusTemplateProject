/**
 * @file SHA256Strategy.cc
 * @brief SHA256Strategy class implementation
 * @details This file contains the implementation of the SHA256Strategy class methods for Cryptographic utilities and toolkit.
 */

#include "SHA256Strategy.hpp"

#include <fmt/format.h>
#include <stdexcept>
#include <glog/logging.h>

namespace common::crypto::hash
{
    SHA256Strategy::SHA256Strategy()
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

    SHA256Strategy::SHA256Strategy(SHA256Strategy&& other)  : ctx_(std::move(other.ctx_)), finalized_(other.finalized_)
    {
        other.finalized_ = true; // Prevent other from being used after move
    }

    SHA256Strategy& SHA256Strategy::operator=(SHA256Strategy&& other)
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

    bool SHA256Strategy::update(const void* data, const size_t length)
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
        finalized_ = false;
        return EVP_DigestInit_ex(ctx_.get(), EVP_sha256(), nullptr) == 1;
    }

    void SHA256Strategy::EvpDeleter::operator()(EVP_MD_CTX* ctx) const
    {
        if (ctx != nullptr)
        {
            EVP_MD_CTX_free(ctx);
        }
    }

    void SHA256Strategy::validateContext() const
    {
        if (!ctx_)
        {
            throw std::runtime_error("Failed to allocate EVP_MD_CTX");
        }
    }
}