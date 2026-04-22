#include "SHA256Strategy.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <stdexcept>

namespace common::crypto::hash {
    SHA256Strategy::SHA256Strategy() {
        if (!ctx_) {
            DLOG(ERROR) << "SHA256Strategy initialization failed - cannot allocate EVP_MD_CTX";
            throw std::runtime_error("Failed to allocate EVP_MD_CTX");
        }
        if (EVP_DigestInit_ex(ctx_.get(), EVP_sha256(), nullptr) != 1) {
            DLOG(ERROR) << "SHA256Strategy initialization failed - OpenSSL DigestInit error";
            throw std::runtime_error("Failed to initialize SHA-256 context");
        }
        DLOG(INFO) << "SHA256Strategy initialized successfully";
    }

    SHA256Strategy::SHA256Strategy(SHA256Strategy &&other) noexcept 
        : ctx_(std::move(other.ctx_)), finalized_(other.finalized_) {
        other.finalized_ = true; // Prevent other from being used after move
    }

    auto SHA256Strategy::operator=(SHA256Strategy &&other) noexcept -> SHA256Strategy & {
        if (this != &other) {
            ctx_ = std::move(other.ctx_);
            finalized_ = other.finalized_;
            other.finalized_ = true; // Prevent other from being used after move
        }
        return *this;
    }

    SHA256Strategy::~SHA256Strategy() = default;

    auto SHA256Strategy::getDigestSize() const noexcept -> size_t {
        return DIGEST_SIZE;
    }

    auto SHA256Strategy::getHexDigestSize() const noexcept -> size_t {
        return HEX_DIGEST_SIZE;
    }

    auto SHA256Strategy::update(const void *data, const size_t length) noexcept -> bool {
        if (finalized_) {
            DLOG(WARNING) << "SHA256Strategy update failed - already finalized";
            return false;
        }
        const bool result = EVP_DigestUpdate(ctx_.get(), data, length) == 1;
        if (!result) {
            DLOG(ERROR) << "SHA256Strategy update failed - OpenSSL DigestUpdate error";
        }
        return result;
    }

    auto SHA256Strategy::finalize() noexcept -> std::optional<std::vector<uint8_t>> {
        if (finalized_) {
            DLOG(WARNING) << "SHA256Strategy finalize called on already finalized instance";
            return std::nullopt;
        }

        std::vector<uint8_t> digest(DIGEST_SIZE);
        unsigned int length = 0;

        if (EVP_DigestFinal_ex(ctx_.get(), digest.data(), &length) != 1) {
            DLOG(ERROR) << "SHA256Strategy finalize failed - OpenSSL DigestFinal error";
            return std::nullopt;
        }

        if (length != DIGEST_SIZE) {
            DLOG(ERROR) << fmt::format("SHA256Strategy finalize failed - unexpected digest length: {} (expected {})", length, DIGEST_SIZE);
            return std::nullopt;
        }

        finalized_ = true;
        DLOG(INFO) << "SHA256Strategy finalized successfully - digest computed";
        return digest;
    }

    auto SHA256Strategy::reset() noexcept -> bool {
        finalized_ = false;
        const bool result = EVP_DigestInit_ex(ctx_.get(), EVP_sha256(), nullptr) == 1;
        if (!result) {
            DLOG(ERROR) << "SHA256Strategy reset failed - OpenSSL DigestInit error";
        } else {
            DLOG(INFO) << "SHA256Strategy reset successfully";
        }
        return result;
    }

    auto SHA256Strategy::EvpDeleter::operator()(EVP_MD_CTX *ctx) const noexcept -> void {
        if (ctx != nullptr) {
            EVP_MD_CTX_free(ctx);
        }
    }

    auto SHA256Strategy::validateContext() const -> void {
        if (!ctx_) {
            throw std::runtime_error("Failed to allocate EVP_MD_CTX");
        }
    }
}
