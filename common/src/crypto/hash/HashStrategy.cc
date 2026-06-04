/**
 * @file HashStrategy.cc
 * @brief HashStrategy class implementation
 * @details This file contains the implementation of the HashStrategy class methods for Cryptographic utilities and toolkit.
 */

#include "HashStrategy.hpp"

#include <array>
#include <fstream>
#include <glog/logging.h>

namespace common::crypto::hash
{
    std::optional<std::string> HashStrategy::toHexString(const std::vector<uint8_t>& digest, const size_t expected_size)
    {
        if (digest.size() != expected_size)
        {
            return std::nullopt;
        }

        static constexpr std::array<char, 16> hex_chars{'0', '1', '2', '3', '4', '5', '6', '7',
                                                        '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
        std::string result(expected_size * 2, '\0');
        for (size_t i = 0; i < expected_size; ++i)
        {
            result[i * 2]     = hex_chars[digest[i] >> 4];
            result[i * 2 + 1] = hex_chars[digest[i] & 0x0F];
        }
        return result;
    }

    std::optional<std::vector<uint8_t>> HashStrategy::hashString(std::unique_ptr<HashStrategy> strategy, const std::string_view input)
    {
        if (!strategy->update(input))
        {
            return std::nullopt;
        }
        return strategy->finalize();
    }

    std::optional<std::vector<uint8_t>> HashStrategy::hashFile(std::unique_ptr<HashStrategy> strategy, const std::string& filePath, const size_t chunkSize)
    {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open())
        {
            LOG(WARNING) << "Failed to open file for hashing: " << filePath;
            return std::nullopt;
        }

        std::vector<char> buffer;
        buffer.resize(chunkSize);

        while (file.read(buffer.data(), static_cast<std::streamsize>(buffer.size())) || file.gcount() > 0)
        {
            const auto bytesRead = file.gcount();
            if (!strategy->update(buffer.data(), static_cast<size_t>(bytesRead)))
            {
                return std::nullopt;
            }
        }

        if (file.bad())
        {
            LOG(WARNING) << "I/O error while reading file: " << filePath;
            return std::nullopt;
        }

        return strategy->finalize();
    }

    std::optional<std::string> HashStrategy::hashStringToHex(std::unique_ptr<HashStrategy> strategy, const std::string_view input)
    {
        const auto digest_size = strategy->getDigestSize();
        const auto digest = hashString(std::move(strategy), input);
        if (!digest)
        {
            return std::nullopt;
        }
        return toHexString(*digest, digest_size);
    }

    std::optional<std::string> HashStrategy::hashFileToHex(std::unique_ptr<HashStrategy> strategy, const std::string& filePath, const size_t chunkSize)
    {
        const auto digest_size = strategy->getDigestSize();
        const auto digest = hashFile(std::move(strategy), filePath, chunkSize);
        if (!digest)
        {
            return std::nullopt;
        }
        return toHexString(*digest, digest_size);
    }
}
