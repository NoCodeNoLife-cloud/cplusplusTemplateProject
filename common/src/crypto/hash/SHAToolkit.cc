#include "SHAToolkit.hpp"
#include "SHA256Strategy.hpp"
#include "SHA1Strategy.hpp"

#include <fmt/format.h>

namespace common::crypto::hash {
    SHAToolkit::SHAToolkit(std::unique_ptr<HashStrategy> strategy) : strategy_(std::move(strategy)) {
    }

    SHAToolkit::SHAToolkit(SHAToolkit &&other) noexcept : strategy_(std::move(other.strategy_)) {
    }

    auto SHAToolkit::operator=(SHAToolkit &&other) noexcept -> SHAToolkit & {
        if (this != &other) {
            strategy_ = std::move(other.strategy_);
        }
        return *this;
    }

    SHAToolkit::~SHAToolkit() = default;

    auto SHAToolkit::getDigestSize() const noexcept -> size_t {
        if (!strategy_) {
            return 0;
        }
        return strategy_->getDigestSize();
    }

    auto SHAToolkit::getHexDigestSize() const noexcept -> size_t {
        if (!strategy_) {
            return 0;
        }
        return strategy_->getHexDigestSize();
    }

    auto SHAToolkit::update(const void *data, const size_t length) noexcept -> bool {
        if (!strategy_) {
            return false;
        }
        return strategy_->update(data, length);
    }

    auto SHAToolkit::update(const std::string_view data) noexcept -> bool {
        if (!strategy_) {
            return false;
        }
        return strategy_->update(data);
    }

    auto SHAToolkit::finalize() noexcept -> std::optional<std::vector<uint8_t>> {
        if (!strategy_) {
            return std::nullopt;
        }
        return strategy_->finalize();
    }

    auto SHAToolkit::reset() noexcept -> bool {
        if (!strategy_) {
            return false;
        }
        return strategy_->reset();
    }

    auto SHAToolkit::toHexString(const std::vector<uint8_t> &digest, const size_t expected_size) -> std::string {
        return HashStrategy::toHexString(digest, expected_size);
    }

    auto SHAToolkit::createSHA256() -> SHAToolkit {
        return SHAToolkit(std::make_unique<SHA256Strategy>());
    }

    auto SHAToolkit::createSHA1() -> SHAToolkit {
        return SHAToolkit(std::make_unique<SHA1Strategy>());
    }

    auto SHAToolkit::hashStringSHA256(const std::string_view input) noexcept -> std::optional<std::vector<uint8_t>> {
        return HashStrategy::hashString(std::make_unique<SHA256Strategy>(), input);
    }

    auto SHAToolkit::hashStringSHA1(const std::string_view input) noexcept -> std::optional<std::vector<uint8_t>> {
        return HashStrategy::hashString(std::make_unique<SHA1Strategy>(), input);
    }

    auto SHAToolkit::hashFileSHA256(const std::string &filePath, const size_t chunkSize) -> std::optional<std::vector<uint8_t>> {
        return HashStrategy::hashFile(std::make_unique<SHA256Strategy>(), filePath, chunkSize);
    }

    auto SHAToolkit::hashFileSHA1(const std::string &filePath, const size_t chunkSize) -> std::optional<std::vector<uint8_t>> {
        return HashStrategy::hashFile(std::make_unique<SHA1Strategy>(), filePath, chunkSize);
    }

    auto SHAToolkit::hashStringToHexSHA256(const std::string_view input) noexcept -> std::string {
        return HashStrategy::hashStringToHex(std::make_unique<SHA256Strategy>(), input);
    }

    auto SHAToolkit::hashStringToHexSHA1(const std::string_view input) noexcept -> std::string {
        return HashStrategy::hashStringToHex(std::make_unique<SHA1Strategy>(), input);
    }

    auto SHAToolkit::hashFileToHexSHA256(const std::string &filePath) -> std::string {
        return HashStrategy::hashFileToHex(std::make_unique<SHA256Strategy>(), filePath);
    }

    auto SHAToolkit::hashFileToHexSHA1(const std::string &filePath) -> std::string {
        return HashStrategy::hashFileToHex(std::make_unique<SHA1Strategy>(), filePath);
    }
}
