/**
 * @file XorBitCipher.cc
 * @brief XorBitCipher implementation â€?repeating-key XOR encryption/decryption
 * @details Implements symmetric XOR cipher: iterates through key bytes and
 *          XORs with plaintext/ciphertext.  Key wraps around when exhausted.
 */

#include <cppforge/crypto/cipher/XorBitCipher.hpp>

#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <glog/logging.h>

namespace cppforge::crypto::cipher
{
    void XorBitCipher::initialize(const std::vector<uint8_t>& key,
                                  const std::vector<uint8_t>& /* nonce */)
    {
        if (key.empty())
        {
            DLOG(WARNING) << "XorBitCipher initialized with empty key";
            throw std::invalid_argument("XorBitCipher requires a non-empty key.");
        }
        key_stream_ = key;
        key_pos_ = 0;
        bit_pos_ = 0;
    }

    std::vector<uint8_t> XorBitCipher::encrypt(const std::vector<uint8_t>& plaintext)
    {
        validateInitialized();
        return process(plaintext);
    }

    std::vector<uint8_t> XorBitCipher::decrypt(const std::vector<uint8_t>& ciphertext)
    {
        validateInitialized();
        // For XOR cipher, decryption is identical to encryption
        return process(ciphertext);
    }

    std::vector<uint8_t> XorBitCipher::generateKeystream(const size_t length)
    {
        validateInitialized();
        std::vector<uint8_t> stream;
        stream.reserve(length);
        for (size_t i = 0; i < length; ++i)
        {
            stream.push_back(nextKeyByte());
        }
        return stream;
    }

    void XorBitCipher::reset()
    {
        key_pos_ = 0;
        bit_pos_ = 0;
    }

    std::string XorBitCipher::getAlgorithmName() const
    {
        return "XorBitCipher";
    }

    bool XorBitCipher::isInitialized() const
    {
        return hasKey();
    }

    std::vector<uint8_t> XorBitCipher::process(const std::vector<uint8_t>& data)
    {
        std::vector<uint8_t> result;
        result.reserve(data.size());

        std::ranges::transform(data, std::back_inserter(result),
                               [this](const uint8_t byte)
                               {
                                   return byte ^ nextKeyByte();
                               });

        return result;
    }

    void XorBitCipher::processInPlace(std::vector<uint8_t>& data)
    {
        for (auto& byte : data)
        {
            byte ^= nextKeyByte();
        }
    }

    std::vector<bool> XorBitCipher::processBits(const std::vector<bool>& bits)
    {
        std::vector<bool> result;
        result.reserve(bits.size());

        std::ranges::transform(bits, std::back_inserter(result),
                               [this](const bool bit)
                               {
                                   return bit ^ nextKeyBit();
                               });

        return result;
    }

    size_t XorBitCipher::getCurrentPosition() const
    {
        return key_pos_;
    }

    uint8_t XorBitCipher::getCurrentBitPosition() const
    {
        return bit_pos_;
    }

    bool XorBitCipher::hasKey() const
    {
        return !key_stream_.empty();
    }

    XorBitCipher XorBitCipher::createWithPseudoRandomKey(const size_t key_length)
    {
        std::vector<uint8_t> random_key;
        random_key.reserve(key_length);

        // Simple deterministic generation for demonstration only
        // In production, use crypto-secure RNG (e.g., std::random_device with proper seeding)
        // Use different seeds based on key_length to ensure different keys for different lengths
        const auto seed = static_cast<uint32_t>(key_length * 2654435761u);
        uint32_t state = seed;
        for (size_t i = 0; i < key_length; ++i)
        {
            // Linear congruential generator with length-dependent seed
            state = state * 1103515245 + 12345;
            random_key.push_back(static_cast<uint8_t>(state >> 16 & 0xFF));
        }

        return XorBitCipher(std::move(random_key));
    }

    void XorBitCipher::validateInitialized() const
    {
        if (!hasKey())
        {
            DLOG(WARNING) << "XorBitCipher operation called without initialization";
            throw std::runtime_error("XorBitCipher not initialized. Call initialize() first.");
        }
    }

    uint8_t XorBitCipher::nextKeyByte()
    {
        if (key_stream_.empty())
        {
            throw std::invalid_argument("Key stream is empty. Set key before processing.");
        }
        const uint8_t byte = key_stream_[key_pos_];
        key_pos_ = (key_pos_ + 1) % key_stream_.size();
        bit_pos_ = 0;
        return byte;
    }

    bool XorBitCipher::nextKeyBit()
    {
        if (key_stream_.empty())
        {
            throw std::invalid_argument("Key stream is empty. Set key before processing.");
        }
        const uint8_t current_byte = key_stream_[key_pos_];
        const bool bit = current_byte >> (MSB_POSITION - bit_pos_) & 0x01;
        bit_pos_++;
        if (bit_pos_ >= BITS_PER_BYTE)
        {
            bit_pos_ = 0;
            key_pos_ = (key_pos_ + 1) % key_stream_.size();
        }
        return bit;
    }
}
