/**
 * @file crypto_benchmark.cc
 * @brief Micro-benchmarks for cryptographic operations
 * @details Measures throughput of PBKDF2 key derivation, AES-256-CBC
 *          encrypt/decrypt, ChaCha20 stream cipher, SHA-256 hashing
 *          (single-shot and chunked), and constant-time secure comparison.
 */

#include <benchmark/benchmark.h>
#include <vector>
#include <string>

#include "crypto/CryptoToolKit.hpp"
#include "crypto/OpenSSLToolkit.hpp"
#include "crypto/cipher/ChaCha20Cipher.hpp"
#include "crypto/hash/SHA256Strategy.hpp"

// ══════════════════════════════════════════════════════════════════════════
//  PBKDF2-HMAC-SHA256 (hash_password)
// ══════════════════════════════════════════════════════════════════════════

/// @brief PBKDF2 with 10 000 iterations (low iteration count).
static void BM_PBKDF2_LowIterations(benchmark::State& state)
{
    const std::string password = "TestPassword123!@#";
    const std::string salt = common::crypto::CryptoToolKit::generate_salt();
    for (auto _ : state)
    {
        (void)_;
        benchmark::DoNotOptimize(common::crypto::CryptoToolKit::hash_password(password, salt, 10000));
    }
}
BENCHMARK(BM_PBKDF2_LowIterations);

/// @brief PBKDF2 with default iteration count (production value).
static void BM_PBKDF2_DefaultIterations(benchmark::State& state)
{
    const std::string password = "TestPassword123!@#";
    const std::string salt = common::crypto::CryptoToolKit::generate_salt();
    for (auto _ : state)
    {
        (void)_;
        benchmark::DoNotOptimize(common::crypto::CryptoToolKit::hash_password(password, salt));
    }
}
BENCHMARK(BM_PBKDF2_DefaultIterations)->Iterations(5);

// ══════════════════════════════════════════════════════════════════════════
//  Secure compare (constant-time)
// ══════════════════════════════════════════════════════════════════════════

/// @brief Constant-time string comparison on 1 KiB inputs.
static void BM_SecureCompare(benchmark::State& state)
{
    const std::string a(1024, 'A');
    const std::string b(1024, 'B');
    for (auto _ : state)
    {
        (void)_;
        bool eq = common::crypto::CryptoToolKit::secure_compare(a, b);
        benchmark::DoNotOptimize(eq);
    }
}
BENCHMARK(BM_SecureCompare);

// ══════════════════════════════════════════════════════════════════════════
//  AES-256-CBC Encrypt / Decrypt
// ══════════════════════════════════════════════════════════════════════════

/// @brief AES-256-CBC encryption throughput at various plaintext sizes.
static void BM_AES256CBC_Encrypt(benchmark::State& state)
{
    const std::string password = "StrongPassword123!";
    const auto data_size = static_cast<size_t>(state.range(0));
    const std::string plaintext(data_size, 'A');

    for (auto _ : state)
    {
        (void)_;
        benchmark::DoNotOptimize(common::crypto::OpenSSLToolkit::encryptAES256CBC(plaintext, password));
    }
}
BENCHMARK(BM_AES256CBC_Encrypt)->Arg(64)->Arg(1024)->Arg(16384);

/// @brief AES-256-CBC decryption throughput (uses pre-encrypted ciphertext).
static void BM_AES256CBC_Decrypt(benchmark::State& state)
{
    const std::string password = "StrongPassword123!";
    const auto data_size = static_cast<size_t>(state.range(0));
    const std::string plaintext(data_size, 'A');
    const auto ciphertext = common::crypto::OpenSSLToolkit::encryptAES256CBC(plaintext, password);

    for (auto _ : state)
    {
        (void)_;
        benchmark::DoNotOptimize(common::crypto::OpenSSLToolkit::decryptAES256CBC(ciphertext, password));
    }
}
BENCHMARK(BM_AES256CBC_Decrypt)->Arg(64)->Arg(1024)->Arg(16384);

// ══════════════════════════════════════════════════════════════════════════
//  ChaCha20 Stream Cipher
// ══════════════════════════════════════════════════════════════════════════

/// @brief Returns a fixed 32-byte ChaCha20 key for reproducible benchmarks.
static std::vector<uint8_t> make_chacha_key()
{
    return std::vector<uint8_t>(32, 0x2B);
}

/// @brief Returns a fixed 12-byte ChaCha20 nonce for reproducible benchmarks.
static std::vector<uint8_t> make_chacha_nonce()
{
    return std::vector<uint8_t>(12, 0x1A);
}

/// @brief ChaCha20 encryption throughput at 64 B, 1 KiB, and 64 KiB payloads.
static void BM_ChaCha20_Encrypt(benchmark::State& state)
{
    const auto data_size = static_cast<size_t>(state.range(0));
    const std::vector<uint8_t> plaintext(data_size, 0x42);
    const auto key = make_chacha_key();
    const auto nonce = make_chacha_nonce();

    for (auto _ : state)
    {
        (void)_;
        auto cipher = common::crypto::cipher::ChaCha20Cipher();
        cipher.initialize(key, nonce);
        benchmark::DoNotOptimize(cipher.encrypt(plaintext));
    }
}
BENCHMARK(BM_ChaCha20_Encrypt)->Arg(64)->Arg(1024)->Arg(65536);

// ══════════════════════════════════════════════════════════════════════════
//  SHA-256 Hashing
// ══════════════════════════════════════════════════════════════════════════

/// @brief SHA-256 single-shot hash throughput from 64 B to 1 MiB.
static void BM_SHA256_SmallData(benchmark::State& state)
{
    const auto data_size = static_cast<size_t>(state.range(0));
    const std::vector<uint8_t> data(data_size, 0xAB);

    for (auto _ : state)
    {
        (void)_;
        common::crypto::hash::SHA256Strategy hasher;
        (void)hasher.update(data.data(), data.size());
        benchmark::DoNotOptimize(hasher.finalize());
    }
}
BENCHMARK(BM_SHA256_SmallData)->Arg(64)->Arg(1024)->Arg(65536)->Arg(1048576);

/// @brief SHA-256 chunked update: 1 MiB total via varying chunk sizes.
static void BM_SHA256_ChunkedUpdate(benchmark::State& state)
{
    constexpr size_t total_size = 1048576;
    const auto chunk_size = static_cast<size_t>(state.range(0));
    const std::vector<uint8_t> data(total_size, 0xCD);

    for (auto _ : state)
    {
        (void)_;
        common::crypto::hash::SHA256Strategy hasher;
        for (size_t offset = 0; offset < total_size; offset += chunk_size)
        {
            (void)hasher.update(data.data() + offset, std::min(chunk_size, total_size - offset));
        }
        benchmark::DoNotOptimize(hasher.finalize());
    }
}
BENCHMARK(BM_SHA256_ChunkedUpdate)->Arg(256)->Arg(4096)->Arg(65536);

// main() provided by benchmark::benchmark_main
