#include "StreamCipherFactory.hpp"
#include "ChaCha20Cipher.hpp"

#include <fmt/format.h>
#include <stdexcept>
#include <vector>
#include <string>

namespace common::crypto::cipher {
    auto StreamCipherFactory::create(Algorithm algo) -> std::unique_ptr<StreamCipher> {
        switch (algo) {
            case Algorithm::CHACHA20:
                return std::make_unique<ChaCha20Cipher>();
            
            default:
                throw std::invalid_argument(
                    "Unsupported stream cipher algorithm: " + 
                    algorithmToString(algo)
                );
        }
    }

    auto StreamCipherFactory::createChaCha20() -> std::unique_ptr<StreamCipher> {
        return create(Algorithm::CHACHA20);
    }

    auto StreamCipherFactory::getSupportedAlgorithms() -> std::vector<std::string> {
        return {"ChaCha20"};
    }

    auto StreamCipherFactory::isSupported(Algorithm algo) noexcept -> bool {
        switch (algo) {
            case Algorithm::CHACHA20:
                return true;
            default:
                return false;
        }
    }

    auto StreamCipherFactory::algorithmToString(Algorithm algo) -> std::string {
        switch (algo) {
            case Algorithm::CHACHA20:
                return "ChaCha20";
            default:
                return "Unknown";
        }
    }
} // namespace common::crypto::cipher
