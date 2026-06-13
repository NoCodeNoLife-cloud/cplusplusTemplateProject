/**
 * @file RandomGenerator.hpp
 * @brief Pseudo-random number generator with multiple distributions
 * @description Wraps C++ <random> facilities (std::mt19937_64 by default) to
 *          provide random integers, floating-point numbers, boolean values,
 *          and byte sequences.  Supports seeding from std::random_device for
 *          non-deterministic initialisation.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Usage Example
 * @code
 * RandomGenerator rng;
 * int dice = rng.nextInt(1, 6);
 * double prob = rng.nextDouble();
 * @endcode
 */

#pragma once

#include <random>
#include <string>
#include <mutex>

namespace common::gen
{
    /// @brief A thread-safe random number generator using Mersenne Twister engine
    /// @details This class provides various methods for generating random numbers,
    ///          including integers, booleans, doubles, strings, and Gaussian distributed values.
    ///          All operations are thread-safe through internal mutex synchronization.
    /// @note For cryptographically secure random numbers, use a dedicated cryptographic library
    class RandomGenerator
    {
    public:
        /// @brief Default character set for string generation
        static constexpr const char* kDefaultCharset = "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "0123456789";

        /// @brief Default constructor: initializes seed using random device
        RandomGenerator();

        /// @brief Constructor with specified seed
        /// @param seed The seed value for the random number generator
        explicit RandomGenerator(unsigned int seed);

        /// @brief Deleted copy constructor (mutex is non-copyable)
        RandomGenerator(const RandomGenerator&) = delete;

        /// @brief Deleted copy assignment operator (mutex is non-copyable)
        RandomGenerator& operator=(const RandomGenerator&) = delete;

        /// @brief Move constructor
        /// @param other The generator to move from
        RandomGenerator(RandomGenerator&& other) noexcept;

        /// @brief Move assignment operator
        /// @param other The generator to move from
        /// @return Reference to this generator
        RandomGenerator& operator=(RandomGenerator&& other) noexcept;

        /// @brief Generates a random integer in the range [min, max]
        /// @param min The minimum value (inclusive)
        /// @param max The maximum value (inclusive)
        /// @return A random integer in the specified range
        /// @throws std::invalid_argument if min > max
        [[nodiscard]] int nextInt(int min, int max);

        /// @brief Generates a random boolean with 50% probability
        /// @return A random boolean value
        [[nodiscard]] bool nextBool();

        /// @brief Generates a random boolean with specified probability
        /// @param trueProbability Probability of returning true (must be between 0.0 and 1.0)
        /// @return A random boolean value
        /// @throws std::invalid_argument if trueProbability is not in [0, 1]
        [[nodiscard]] bool nextBool(double trueProbability);

        /// @brief Generates a random floating-point number in the range [min, max)
        /// @param min The minimum value (inclusive)
        /// @param max The maximum value (exclusive)
        /// @return A random double in the specified range
        /// @throws std::invalid_argument if min >= max
        [[nodiscard]] double nextDouble(double min, double max);

        /// @brief Generates a random string of specified length
        /// @param length The length of the string to generate
        /// @param charset The character set to use for generation (default: alphanumeric)
        /// @return A random string of specified length
        /// @throws std::invalid_argument if charset is empty
        [[nodiscard]] std::string nextString(size_t length, const std::string& charset = kDefaultCharset);

        /// @brief Generates a Gaussian distributed random number
        /// @param mean The mean of the distribution (default: 0.0)
        /// @param stddev The standard deviation of the distribution (must be positive, default: 1.0)
        /// @return A Gaussian distributed random number
        /// @throws std::invalid_argument if stddev is not positive
        [[nodiscard]] double nextGaussian(double mean = 0.0, double stddev = 1.0);

        /// @brief Thread-safe: sets a new seed
        /// @param seed The new seed value
        void setSeed(unsigned int seed);

    private:
        std::mt19937 engine_; ///< Mersenne Twister random number engine
        std::mutex mutex_; ///< Mutex for thread safety
    };
}