#include "HashStrategy.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <fstream>
#include <sstream>
#include <iomanip>

namespace common::crypto::hash {
    auto HashStrategy::toHexString(const std::vector<uint8_t> &digest, const size_t expected_size) -> std::string {
        if (digest.size() != expected_size) {
            DLOG(ERROR) << fmt::format("HashStrategy toHexString failed - digest size mismatch: {} (expected {})", digest.size(), expected_size);
            return "";
        }

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');
        for (const auto byte: digest) {
            oss << std::setw(2) << static_cast<int>(byte);
        }
        return oss.str();
    }

    auto HashStrategy::hashString(std::unique_ptr<HashStrategy> strategy, const std::string_view input) noexcept -> std::optional<std::vector<uint8_t>> {
        DLOG(INFO) << fmt::format("HashStrategy hashString - input length: {}", input.length());
        if (!strategy->update(input)) {
            DLOG(ERROR) << "HashStrategy hashString failed - update error";
            return std::nullopt;
        }
        return strategy->finalize();
    }

    auto HashStrategy::hashFile(std::unique_ptr<HashStrategy> strategy, const std::string &filePath, const size_t chunkSize) -> std::optional<std::vector<uint8_t>> {
        DLOG(INFO) << fmt::format("HashStrategy hashFile - file: {}, chunk size: {}", filePath, chunkSize);
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            DLOG(ERROR) << fmt::format("HashStrategy hashFile failed - cannot open file: {}", filePath);
            return std::nullopt;
        }

        std::vector<char> buffer(chunkSize);

        while (file.good()) {
            file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
            const std::streamsize bytesRead = file.gcount();
            if (bytesRead > 0) {
                if (!strategy->update(buffer.data(), static_cast<size_t>(bytesRead))) {
                    DLOG(ERROR) << fmt::format("HashStrategy hashFile failed - update error after reading {} bytes", bytesRead);
                    return std::nullopt;
                }
            }
        }

        if (file.bad()) {
            DLOG(ERROR) << fmt::format("HashStrategy hashFile failed - file read error: {}", filePath);
            return std::nullopt;
        }

        DLOG(INFO) << fmt::format("HashStrategy hashFile completed - file: {}", filePath);
        return strategy->finalize();
    }

    auto HashStrategy::hashStringToHex(std::unique_ptr<HashStrategy> strategy, const std::string_view input) noexcept -> std::string {
        const auto digest_size = strategy->getDigestSize();
        const auto digest = hashString(std::move(strategy), input);
        if (!digest) {
            return "";
        }
        return toHexString(*digest, digest_size);
    }

    auto HashStrategy::hashFileToHex(std::unique_ptr<HashStrategy> strategy, const std::string &filePath) -> std::string {
        const auto digest_size = strategy->getDigestSize();
        const auto digest = hashFile(std::move(strategy), filePath);
        if (!digest) {
            return "";
        }
        return toHexString(*digest, digest_size);
    }
}
