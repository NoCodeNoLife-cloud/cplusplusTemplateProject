/**
 * @file SHAToolkit.cc
 * @brief SHAToolkit class implementation
 * @details This file contains the implementation of the SHAToolkit class methods for Cryptographic utilities and toolkit.
 */

#include "SHAToolkit.hpp"

#include <fmt/format.h>
#include <glog/logging.h>

#include "SHA1Strategy.hpp"
#include "SHA256Strategy.hpp"

namespace common::crypto::hash
{
    SHAToolkit::SHAToolkit(std::unique_ptr<HashStrategy> strategy) : strategy_(std::move(strategy))
    {
    }

    SHAToolkit::SHAToolkit(SHAToolkit&& other)  : strategy_(std::move(other.strategy_))
    {
    }

    SHAToolkit& SHAToolkit::operator=(SHAToolkit&& other)
    {
        if (this != &other)
        {
            strategy_ = std::move(other.strategy_);
        }
        return *this;
    }

    SHAToolkit::~SHAToolkit() = default;

    size_t SHAToolkit::getDigestSize() const
    {
        if (!strategy_)
        {
            return 0;
        }
        return strategy_->getDigestSize();
    }

    size_t SHAToolkit::getHexDigestSize() const
    {
        if (!strategy_)
        {
            return 0;
        }
        return strategy_->getHexDigestSize();
    }

    bool SHAToolkit::update(const void* data, const size_t length)
    {
        if (!strategy_)
        {
            DLOG(WARNING) << "SHAToolkit update called without valid strategy";
            return false;
        }
        return strategy_->update(data, length);
    }

    bool SHAToolkit::update(const std::string_view data)
    {
        if (!strategy_)
        {
            return false;
        }
        return strategy_->update(data);
    }

    std::optional<std::vector<uint8_t>> SHAToolkit::finalize()
    {
        if (!strategy_)
        {
            DLOG(WARNING) << "SHAToolkit finalize called without valid strategy";
            return std::nullopt;
        }
        return strategy_->finalize();
    }

    bool SHAToolkit::reset()
    {
        if (!strategy_)
        {
            return false;
        }
        return strategy_->reset();
    }

    std::optional<std::string> SHAToolkit::toHexString(const std::vector<uint8_t>& digest, const size_t expected_size)
    {
        return HashStrategy::toHexString(digest, expected_size);
    }

    SHAToolkit SHAToolkit::createSHA256()
    {
        return SHAToolkit(std::make_unique<SHA256Strategy>());
    }

    SHAToolkit SHAToolkit::createSHA1()
    {
        return SHAToolkit(std::make_unique<SHA1Strategy>());
    }

    std::optional<std::vector<uint8_t>> SHAToolkit::hashStringSHA256(const std::string_view input)
    {
        return HashStrategy::hashString(std::make_unique<SHA256Strategy>(), input);
    }

    std::optional<std::vector<uint8_t>> SHAToolkit::hashStringSHA1(const std::string_view input)
    {
        return HashStrategy::hashString(std::make_unique<SHA1Strategy>(), input);
    }

    std::optional<std::vector<uint8_t>> SHAToolkit::hashFileSHA256(const std::string& filePath, const size_t chunkSize)
    {
        return HashStrategy::hashFile(std::make_unique<SHA256Strategy>(), filePath, chunkSize);
    }

    std::optional<std::vector<uint8_t>> SHAToolkit::hashFileSHA1(const std::string& filePath, const size_t chunkSize)
    {
        return HashStrategy::hashFile(std::make_unique<SHA1Strategy>(), filePath, chunkSize);
    }

    std::optional<std::string> SHAToolkit::hashStringToHexSHA256(const std::string_view input)
    {
        return HashStrategy::hashStringToHex(std::make_unique<SHA256Strategy>(), input);
    }

    std::optional<std::string> SHAToolkit::hashStringToHexSHA1(const std::string_view input)
    {
        return HashStrategy::hashStringToHex(std::make_unique<SHA1Strategy>(), input);
    }

    std::optional<std::string> SHAToolkit::hashFileToHexSHA256(const std::string& filePath, const size_t chunkSize)
    {
        return HashStrategy::hashFileToHex(std::make_unique<SHA256Strategy>(), filePath, chunkSize);
    }

    std::optional<std::string> SHAToolkit::hashFileToHexSHA1(const std::string& filePath, const size_t chunkSize)
    {
        return HashStrategy::hashFileToHex(std::make_unique<SHA1Strategy>(), filePath, chunkSize);
    }
}
