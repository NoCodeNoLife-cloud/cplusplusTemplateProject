#include "XorBitCipher.hpp"

#include <stdexcept>

namespace common::crypto::cipher {
    auto XorBitCipher::setKey(std::vector<uint8_t> key) noexcept -> XorBitCipher & {
        key_stream_ = std::move(key);
        key_pos_ = 0;
        bit_pos_ = 0;
        return *this;
    }

    auto XorBitCipher::resetPosition() noexcept -> XorBitCipher & {
        key_pos_ = 0;
        bit_pos_ = 0;
        return *this;
    }

    auto XorBitCipher::getCurrentPosition() const noexcept -> size_t {
        return key_pos_;
    }

    auto XorBitCipher::hasKey() const noexcept -> bool {
        return !key_stream_.empty();
    }

    auto XorBitCipher::nextKeyByte() const -> uint8_t {
        if (key_stream_.empty()) {
            throw std::invalid_argument("Key stream is empty. Set key before processing.");
        }
        const uint8_t byte = key_stream_[key_pos_];
        key_pos_ = (key_pos_ + 1) % key_stream_.size();
        bit_pos_ = 0;
        return byte;
    }

    auto XorBitCipher::nextKeyBit() const -> bool {
        if (key_stream_.empty()) {
            throw std::invalid_argument("Key stream is empty. Set key before processing.");
        }
        const uint8_t current_byte = key_stream_[key_pos_];
        const bool bit = (current_byte >> (7 - bit_pos_)) & 0x01;
        bit_pos_++;
        if (bit_pos_ >= 8) {
            bit_pos_ = 0;
            key_pos_ = (key_pos_ + 1) % key_stream_.size();
        }
        return bit;
    }

    auto XorBitCipher::process(const std::vector<uint8_t> &data) const -> std::vector<uint8_t> {
        std::vector<uint8_t> result;
        result.reserve(data.size());

        for (const uint8_t byte: data) {
            result.push_back(byte ^ nextKeyByte());
        }
        return result;
    }

    auto XorBitCipher::processInPlace(std::vector<uint8_t> &data) const -> void {
        for (auto &byte: data) {
            byte ^= nextKeyByte();
        }
    }

    auto XorBitCipher::processBits(const std::vector<bool> &bits) const -> std::vector<bool> {
        std::vector<bool> result;
        result.reserve(bits.size());

        for (const bool bit: bits) {
            result.push_back(bit ^ nextKeyBit());
        }
        return result;
    }

    auto XorBitCipher::generateKeyStream(const size_t length) const -> std::vector<uint8_t> {
        std::vector<uint8_t> stream;
        stream.reserve(length);

        for (size_t i = 0; i < length; ++i) {
            stream.push_back(nextKeyByte());
        }
        return stream;
    }

    auto XorBitCipher::createWithRandomKey(const size_t key_length) -> XorBitCipher {
        std::vector<uint8_t> random_key;
        random_key.reserve(key_length);

        // Simple deterministic generation for demonstration only
        // In production, use crypto-secure RNG (e.g., std::random_device with proper seeding)
        // Use different seeds based on key_length to ensure different keys for different lengths
        const uint32_t seed = static_cast<uint32_t>(key_length * 2654435761u);
        uint32_t state = seed;
        for (size_t i = 0; i < key_length; ++i) {
            // Linear congruential generator with length-dependent seed
            state = state * 1103515245 + 12345;
            random_key.push_back(static_cast<uint8_t>((state >> 16) & 0xFF));
        }

        return XorBitCipher(std::move(random_key));
    }
} // namespace common::crypto::cipher
