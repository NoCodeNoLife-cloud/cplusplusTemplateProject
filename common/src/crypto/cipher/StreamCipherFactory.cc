/**
* @file StreamCipherFactory.cc
 * @brief StreamCipherFactory class implementation
 * @details This file contains the implementation of the StreamCipherFactory class methods for Cryptographic utilities and toolkit.
 */

#include "StreamCipherFactory.hpp"
#include "ChaCha20Cipher.hpp"

#include <fmt/format.h>
#include <stdexcept>
#include <vector>
#include <string>
#include <glog/logging.h>

namespace common::crypto::cipher
{
    std::unique_ptr<StreamCipher> StreamCipherFactory::create(Algorithm algo)
    {
        switch (algo)
        {
        case Algorithm::CHACHA20:
            return std::make_unique<ChaCha20Cipher>();

        default:
            DLOG(WARNING) << fmt::format("Unsupported stream cipher algorithm requested: {}", algorithmToString(algo));
            throw std::invalid_argument(
                "Unsupported stream cipher algorithm: " +
                algorithmToString(algo)
            );
        }
    }

    std::unique_ptr<StreamCipher> StreamCipherFactory::createChaCha20()
    {
        return create(Algorithm::CHACHA20);
    }

    std::vector<std::string> StreamCipherFactory::getSupportedAlgorithms()
    {
        return {"ChaCha20"};
    }

    bool StreamCipherFactory::isSupported(Algorithm algo) noexcept
    {
        switch (algo)
        {
        case Algorithm::CHACHA20:
            return true;
        default:
            return false;
        }
    }

    std::string StreamCipherFactory::algorithmToString(Algorithm algo)
    {
        switch (algo)
        {
        case Algorithm::CHACHA20:
            return "ChaCha20";
        default:
            return "Unknown";
        }
    }
}