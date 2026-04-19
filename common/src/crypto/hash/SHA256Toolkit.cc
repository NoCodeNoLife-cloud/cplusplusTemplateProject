#include "SHA256Toolkit.hpp"

#include <fstream>
#include <sstream>

common::crypto::hash::SHA256Toolkit::SHA256Toolkit() {
    if (EVP_DigestInit_ex(ctx_.get(), EVP_sha256(), nullptr) != 1) {
        throw std::runtime_error("Failed to initialize SHA-256 context");
    }
}

common::crypto::hash::SHA256Toolkit::SHA256Toolkit(SHA256Toolkit &&) noexcept = default;

auto common::crypto::hash::SHA256Toolkit::operator=(SHA256Toolkit &&) noexcept -> SHA256Toolkit & = default;

common::crypto::hash::SHA256Toolkit::~SHA256Toolkit() = default;

auto common::crypto::hash::SHA256Toolkit::update(const void *data, size_t length) const noexcept -> bool {
    if (finalized_) {
        return false;
    }
    return EVP_DigestUpdate(ctx_.get(), data, length) == 1;
}

auto common::crypto::hash::SHA256Toolkit::update(std::string_view data) const noexcept -> bool {
    return update(data.data(), data.size());
}

auto common::crypto::hash::SHA256Toolkit::finalize() noexcept -> std::optional<std::vector<uint8_t> > {
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

auto common::crypto::hash::SHA256Toolkit::reset() noexcept -> bool {
    finalized_ = false;
    return EVP_DigestInit_ex(ctx_.get(), EVP_sha256(), nullptr) == 1;
}

auto common::crypto::hash::SHA256Toolkit::toHexString(const std::vector<uint8_t> &digest) -> std::string {
    if (digest.size() != DIGEST_SIZE) {
        return "";
    }

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (const auto byte: digest) {
        oss << std::setw(2) << static_cast<int>(byte);
    }
    return oss.str();
}

auto common::crypto::hash::SHA256Toolkit::hashString(std::string_view input) noexcept -> std::optional<std::vector<uint8_t> > {
    SHA256Toolkit toolkit;
    if (!toolkit.update(input)) {
        return std::nullopt;
    }
    return toolkit.finalize();
}

auto common::crypto::hash::SHA256Toolkit::hashFile(const std::string &filePath, const size_t chunkSize) -> std::optional<std::vector<uint8_t> > {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return std::nullopt;
    }

    SHA256Toolkit toolkit;
    std::vector<char> buffer(chunkSize);

    while (file.good()) {
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize bytesRead = file.gcount();
        if (bytesRead > 0) {
            if (!toolkit.update(buffer.data(), static_cast<size_t>(bytesRead))) {
                return std::nullopt;
            }
        }
    }

    if (file.bad()) {
        return std::nullopt;
    }

    return toolkit.finalize();
}

auto common::crypto::hash::SHA256Toolkit::hashStringToHex(std::string_view input) noexcept -> std::string {
    const auto digest = hashString(input);
    if (!digest) {
        return "";
    }
    return toHexString(*digest);
}

auto common::crypto::hash::SHA256Toolkit::hashFileToHex(const std::string &filePath) -> std::string {
    const auto digest = hashFile(filePath);
    if (!digest) {
        return "";
    }
    return toHexString(*digest);
}

auto common::crypto::hash::SHA256Toolkit::EvpDeleter::operator()(EVP_MD_CTX *ctx) const noexcept -> void {
    if (ctx != nullptr) {
        EVP_MD_CTX_free(ctx);
    }
}

auto common::crypto::hash::SHA256Toolkit::validateContext() const -> void {
    if (!ctx_) {
        throw std::runtime_error("Failed to allocate EVP_MD_CTX");
    }
}
