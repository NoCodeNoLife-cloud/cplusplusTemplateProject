/**
 * @file RandomGenerator.cc
 * @brief RandomGenerator implementation â€?Mersenne Twister distribution wrappers
 * @details Implements PRNG methods wrapping std::mt19937_64 with uniform,
 *          normal, and bernoulli distributions.  Seeds from std::random_device
 *          for non-deterministic initialisation.
 */

#include <cppforge/gen/RandomGenerator.hpp>
#include <fmt/format.h>
#include <random>
#include <string>
#include <stdexcept>
#include <mutex>
#include <glog/logging.h>

namespace cppforge::gen
{
    RandomGenerator::RandomGenerator() : engine_(std::random_device{}())
    {
    }

    RandomGenerator::RandomGenerator(const unsigned int seed) : engine_(seed)
    {
    }

    RandomGenerator::RandomGenerator(RandomGenerator&& other) noexcept
        : engine_(std::move(other.engine_))
    {
    }

    RandomGenerator& RandomGenerator::operator=(RandomGenerator&& other) noexcept
    {
        if (this != &other)
        {
            std::lock_guard lock(mutex_);
            engine_ = std::move(other.engine_);
        }
        return *this;
    }

    int RandomGenerator::nextInt(const int min, const int max)
    {
        if (min > max)
        {
            DLOG(WARNING) << fmt::format("RandomGenerator nextInt: min ({}) > max ({})", min, max);
            throw std::invalid_argument("cppforge::RandomGenerator::nextInt: min cannot be greater than max");
        }
        std::lock_guard lock(mutex_);
        std::uniform_int_distribution dist(min, max);
        return dist(engine_);
    }

    bool RandomGenerator::nextBool()
    {
        std::lock_guard lock(mutex_);
        std::uniform_int_distribution<int> dist(0, 1);
        return dist(engine_) == 1;
    }

    bool RandomGenerator::nextBool(const double trueProbability)
    {
        if (trueProbability < 0.0 || trueProbability > 1.0)
        {
            DLOG(WARNING) << fmt::format("RandomGenerator nextBool: probability {} out of range [0, 1]", trueProbability);
            throw std::invalid_argument("cppforge::RandomGenerator::nextBool: probability must be in [0, 1]");
        }
        std::lock_guard lock(mutex_);
        if (trueProbability == 0.0)
        {
            return false;
        }
        if (trueProbability == 1.0)
        {
            return true;
        }
        std::bernoulli_distribution dist(trueProbability);
        return dist(engine_);
    }

    double RandomGenerator::nextDouble(const double min, const double max)
    {
        if (min >= max)
        {
            DLOG(WARNING) << fmt::format("RandomGenerator nextDouble: min ({}) >= max ({})", min, max);
            throw std::invalid_argument("cppforge::RandomGenerator::nextDouble: min must be less than max");
        }
        std::lock_guard lock(mutex_);
        std::uniform_real_distribution dist(min, max);
        return dist(engine_);
    }

    std::string RandomGenerator::nextString(const size_t length, const std::string& charset)
    {
        if (charset.empty())
        {
            DLOG(WARNING) << "RandomGenerator nextString: charset is empty";
            throw std::invalid_argument("cppforge::RandomGenerator::nextString: charset cannot be empty");
        }
        if (length == 0)
        {
            return {};
        }

        std::lock_guard lock(mutex_);
        std::string result;
        result.resize(length);

        const size_t charSetSize = charset.size();
        std::uniform_int_distribution<size_t> dist(0, charSetSize - 1);

        for (size_t i = 0; i < length; ++i)
        {
            result[i] = charset[dist(engine_)];
        }
        return result;
    }

    double RandomGenerator::nextGaussian(const double mean, const double stddev)
    {
        if (stddev <= 0.0)
        {
            DLOG(WARNING) << fmt::format("RandomGenerator nextGaussian: stddev ({}) must be positive", stddev);
            throw std::invalid_argument("cppforge::RandomGenerator::nextGaussian: stddev must be positive");
        }
        std::lock_guard lock(mutex_);
        std::normal_distribution dist(mean, stddev);
        return dist(engine_);
    }

    void RandomGenerator::setSeed(const unsigned int seed)
    {
        std::lock_guard lock(mutex_);
        engine_.seed(seed);
    }
}
