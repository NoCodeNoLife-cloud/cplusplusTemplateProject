#include "SHA1Strategy.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <stdexcept>

namespace common::crypto::hash {
    SHA1Strategy::SHA1Strategy() {
        if (!ctx_) {
            DLOG(ERROR) << "SHA1Strategy initialization failed - cannot allocate EVP_MD_CTX";
            throw std::runtime_error("Failed to allocate EVP_MD_CTX");
        }
        if (EVP_DigestInit_ex(ctx_.get(), EVP_sha1(), nullptr) != 1) {
            DLOG(ERROR) << "SHA1Strategy initialization failed - OpenSSL DigestInit error";
            throw std::runtime_error("Failed to initialize SHA-1 context");
        }
        DLOG(INFO) << "SHA1Strategy initialized successfully";
    }

    SHA1Strategy::SHA1Strategy(SHA1Strategy &&other) noexcept 
        : ctx_(std::move(other.ctx_)), finalized_(other.finalized_) {
        other.finalized_ = true; // Prevent other from being used after move
    }

    auto SHA1Strategy::operator=(SHA1Strategy &&other) noexcept -> SHA1Strategy & {
        if (this != &other) {
            ctx_ = std::move(other.ctx_);
            finalized_ = other.finalized_;
            other.finalized_ = true; // Prevent other from being used after move
        }
        return *this;
    }

    SHA1Strategy::~SHA1Strategy() = default;

    auto SHA1Strategy::getDigestSize() const noexcept -> size_t {
        return DIGEST_SIZE;
    }

    auto SHA1Strategy::getHexDigestSize() const noexcept -> size_t {
        return HEX_DIGEST_SIZE;
    }

    auto SHA1Strategy::update(const void *data, const size_t length) noexcept -> bool {
        if (finalized_) {
            DLOG(WARNING) << "SHA1Strategy update failed - already finalized";
            return false;
        }
        const bool result = EVP_DigestUpdate(ctx_.get(), data, length) == 1;
        if (!result) {
            DLOG(ERROR) << "SHA1Strategy update failed - OpenSSL DigestUpdate error";
        }
        return result;
    }

    auto SHA1Strategy::finalize() noexcept -> std::optional<std::vector<uint8_t>> {
        if (finalized_) {
            DLOG(WARNING) << "SHA1Strategy finalize called on already finalized instance";
            return std::nullopt;
        }

        std::vector<uint8_t> digest(DIGEST_SIZE);
        unsigned int length = 0;

        if (EVP_DigestFinal_ex(ctx_.get(), digest.data(), &length) != 1) {
            DLOG(ERROR) << "SHA1Strategy finalize failed - OpenSSL DigestFinal error";
            return std::nullopt;
        }

        if (length != DIGEST_SIZE) {
            DLOG(ERROR) << fmt::format("SHA1Strategy finalize failed - unexpected digest length: {} (expected {})", length, DIGEST_SIZE);
            return std::nullopt;
        }

        finalized_ = true;
        DLOG(INFO) << "SHA1Strategy finalized successfully - digest computed";
        return digest;
    }

    auto SHA1Strategy::reset() noexcept -> bool {
        finalized_ = false;
        const bool result = EVP_DigestInit_ex(ctx_.get(), EVP_sha1(), nullptr) == 1;
        if (!result) {
            DLOG(ERROR) << "SHA1Strategy reset failed - OpenSSL DigestInit error";
        } else {
            DLOG(INFO) << "SHA1Strategy reset successfully";
        }
        return result;
    }

    auto SHA1Strategy::EvpDeleter::operator()(EVP_MD_CTX *ctx) const noexcept -> void {
        if (ctx != nullptr) {
            EVP_MD_CTX_free(ctx);
        }
    }

    auto SHA1Strategy::validateContext() const -> void {
        if (!ctx_) {
            throw std::runtime_error("Failed to allocate EVP_MD_CTX");
        }
    }
}
