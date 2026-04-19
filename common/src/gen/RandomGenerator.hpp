#pragma once

#include <random>
#include <string>
#include <mutex>

namespace common::gen {
    /// @brief A thread-safe random number generator using Mersenne Twister engine
    /// @details This class provides various methods for generating random numbers,
    ///          including integers, booleans, doubles, strings, and Gaussian distributed values.
    ///          All operations are thread-safe through internal mutex synchronization.
    /// @note For cryptographically secure random numbers, use a dedicated cryptographic library
    class RandomGenerator {
    public:
        /// @brief Default constructor: initializes seed using random device
        RandomGenerator() noexcept;

        /// @brief Constructor with specified seed
        /// @param seed The seed value for the random number generator
        explicit RandomGenerator(unsigned int seed) noexcept;

        /// @brief Generates a random integer in the range [min, max]
        /// @param min The minimum value (inclusive)
        /// @param max The maximum value (inclusive)
        /// @return A random integer in the specified range
        /// @throws std::invalid_argument if min > max
        [[nodiscard]] auto nextInt(int min, int max) -> int;

        /// @brief Generates a random boolean with 50% probability
        /// @return A random boolean value
        [[nodiscard]] auto nextBool() -> bool;

        /// @brief Generates a random boolean with specified probability
        /// @param trueProbability Probability of returning true (must be between 0.0 and 1.0)
        /// @return A random boolean value
        /// @throws std::invalid_argument if trueProbability is not in [0, 1]
        [[nodiscard]] auto nextBool(double trueProbability) -> bool;

        /// @brief Generates a random floating-point number in the range [min, max)
        /// @param min The minimum value (inclusive)
        /// @param max The maximum value (exclusive)
        /// @return A random double in the specified range
        /// @throws std::invalid_argument if min >= max
        [[nodiscard]] auto nextDouble(double min, double max) -> double;

        /// @brief Generates a random string of specified length
        /// @param length The length of the string to generate
        /// @param charset The character set to use for generation
        /// @return A random string of specified length
        /// @throws std::invalid_argument if charset is empty
        [[nodiscard]] auto nextString(size_t length, const std::string &charset = "abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "0123456789") -> std::string;

        /// @brief Generates a Gaussian distributed random number
        /// @param mean The mean of the distribution
        /// @param stddev The standard deviation of the distribution (must be positive)
        /// @return A Gaussian distributed random number
        /// @throws std::invalid_argument if stddev is not positive
        [[nodiscard]] auto nextGaussian(double mean = 0.0, double stddev = 1.0) -> double;

        /// @brief Thread-safe: sets a new seed
        /// @param seed The new seed value
        auto setSeed(unsigned int seed) -> void;

    private:
        std::mt19937 engine_; ///< Mersenne Twister random number engine
        std::mutex mutex_; ///< Mutex for thread safety
    };
}
