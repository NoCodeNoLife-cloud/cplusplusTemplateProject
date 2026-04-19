#include "SHA1Strategy.hpp"

#include <stdexcept>

namespace common::crypto::hash {
    SHA1Strategy::SHA1Strategy() {
        if (!ctx_) {
            throw std::runtime_error("Failed to allocate EVP_MD_CTX");
        }
        if (EVP_DigestInit_ex(ctx_.get(), EVP_sha1(), nullptr) != 1) {
            throw std::runtime_error("Failed to initialize SHA-1 context");
        }
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
            return false;
        }
        return EVP_DigestUpdate(ctx_.get(), data, length) == 1;
    }

    auto SHA1Strategy::finalize() noexcept -> std::optional<std::vector<uint8_t>> {
        if (finalized_) {
            return std::nullopt;
        }

        std::vector<uint8_t> digest(DIGEST_SIZE);
        unsigned int length = 0;

        if (EVP_DigestFinal_ex(ctx_.get(), digest.data(), &length) != 1) {
            return std::nullopt;
        }

        if (length != DIGEST_SIZE) {
            return std::nullopt;
        }

        finalized_ = true;
        return digest;
    }

    auto SHA1Strategy::reset() noexcept -> bool {
        finalized_ = false;
        return EVP_DigestInit_ex(ctx_.get(), EVP_sha1(), nullptr) == 1;
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
