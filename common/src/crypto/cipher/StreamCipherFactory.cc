/**
 * @file StreamCipherFactory.cc
 * @brief StreamCipherFactory implementation — cipher type registration and creation
 * @details Implements factory dispatch: maps cipher type strings ("caesar",
 *          "xor", "chacha20") to concrete StreamCipher constructors.
 */

#include "StreamCipherFactory.hpp"

#include <stdexcept>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <glog/logging.h>

#include "ChaCha20Cipher.hpp"

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

    bool StreamCipherFactory::isSupported(Algorithm algo)
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
