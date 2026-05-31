/**
 * @file XorBitCipher.cc
 * @brief XorBitCipher class implementation
 * @details This file contains the implementation of the XorBitCipher class methods for Cryptographic utilities and toolkit.
 */

#include "XorBitCipher.hpp"

#include <fmt/format.h>
#include <stdexcept>
#include <string>
#include <ranges>
#include <algorithm>
#include <glog/logging.h>

namespace common::crypto::cipher
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
        return generateKeyStream(length);
    }

    void XorBitCipher::reset()
    {
        key_pos_ = 0;
        bit_pos_ = 0;
    }

    std::string XorBitCipher::getAlgorithmName() const noexcept
    {
        return "XorBitCipher";
    }

    bool XorBitCipher::isInitialized() const noexcept
    {
        return hasKey();
    }

    void XorBitCipher::validateInitialized() const
    {
        if (!hasKey())
        {
            DLOG(WARNING) << "XorBitCipher operation called without initialization";
            throw std::runtime_error("XorBitCipher not initialized. Call initialize() first.");
        }
    }

    size_t XorBitCipher::getCurrentPosition() const noexcept
    {
        return key_pos_;
    }

    bool XorBitCipher::hasKey() const noexcept
    {
        return !key_stream_.empty();
    }

    uint8_t XorBitCipher::nextKeyByte() const
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

    bool XorBitCipher::nextKeyBit() const
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

    std::vector<uint8_t> XorBitCipher::process(const std::vector<uint8_t>& data) const
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

    void XorBitCipher::processInPlace(std::vector<uint8_t>& data) const
    {
        for (auto& byte : data)
        {
            byte ^= nextKeyByte();
        }
    }

    std::vector<bool> XorBitCipher::processBits(const std::vector<bool>& bits) const
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

    std::vector<uint8_t> XorBitCipher::generateKeyStream(const size_t length) const
    {
        std::vector<uint8_t> stream;
        stream.reserve(length);

        for (size_t i = 0; i < length; ++i)
        {
            stream.push_back(nextKeyByte());
        }
        return stream;
    }

    XorBitCipher XorBitCipher::createWithRandomKey(const size_t key_length)
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
}