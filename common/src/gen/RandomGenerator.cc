/**
 * @file RandomGenerator.cc
 * @brief RandomGenerator class implementation
 * @details This file contains the implementation of the RandomGenerator class methods for Random number and ID generation utilities.
 */

#include "gen/RandomGenerator.hpp"

#include <fmt/format.h>
#include <random>
#include <string>
#include <stdexcept>
#include <mutex>
#include <glog/logging.h>

namespace common::gen
{
    RandomGenerator::RandomGenerator()  : engine_(std::random_device{}())
    {
    }

    RandomGenerator::RandomGenerator(const unsigned int seed)  : engine_(seed)
    {
    }

    int RandomGenerator::nextInt(const int min, const int max)
    {
        if (min > max)
        {
            DLOG(WARNING) << fmt::format("RandomGenerator nextInt: min ({}) > max ({})", min, max);
            throw std::invalid_argument("common::RandomGenerator::nextInt: min cannot be greater than max");
        }
        std::lock_guard lock(mutex_);
        std::uniform_int_distribution dist(min, max);
        const auto result = dist(engine_);
        return result;
    }

    bool RandomGenerator::nextBool()
    {
        return nextBool(0.5);
    }

    bool RandomGenerator::nextBool(const double trueProbability)
    {
        if (trueProbability < 0.0 || trueProbability > 1.0)
        {
            DLOG(WARNING) << fmt::format("RandomGenerator nextBool: probability {} out of range [0, 1]", trueProbability);
            throw std::invalid_argument("common::RandomGenerator::nextBool: probability must be in [0, 1]");
        }
        std::lock_guard lock(mutex_);
        std::bernoulli_distribution dist(trueProbability);
        const auto result = dist(engine_);
        return result;
    }

    double RandomGenerator::nextDouble(const double min, const double max)
    {
        if (min >= max)
        {
            DLOG(WARNING) << fmt::format("RandomGenerator nextDouble: min ({}) >= max ({})", min, max);
            throw std::invalid_argument("common::RandomGenerator::nextDouble: min must be less than max");
        }
        std::lock_guard lock(mutex_);
        std::uniform_real_distribution dist(min, max);
        const auto result = dist(engine_);
        return result;
    }

    std::string RandomGenerator::nextString(const size_t length, const std::string& charset)
    {
        if (charset.empty())
        {
            DLOG(WARNING) << "RandomGenerator nextString: charset is empty";
            throw std::invalid_argument("common::RandomGenerator::nextString: charset cannot be empty");
        }
        if (length == 0)
        {
            return {};
        }

        std::lock_guard lock(mutex_);
        std::string result;
        result.reserve(length);

        // Pre-calculate charset size to avoid repeated size() calls
        const size_t charSetSize = charset.size();
        std::uniform_int_distribution<size_t> dist(0, charSetSize - 1);

        for (size_t i = 0; i < length; ++i)
        {
            result += charset[dist(engine_)];
        }
        return result;
    }

    double RandomGenerator::nextGaussian(const double mean, const double stddev)
    {
        if (stddev <= 0.0)
        {
            DLOG(WARNING) << fmt::format("RandomGenerator nextGaussian: stddev ({}) must be positive", stddev);
            throw std::invalid_argument("common::RandomGenerator::nextGaussian: stddev must be positive");
        }
        std::lock_guard lock(mutex_);
        std::normal_distribution dist(mean, stddev);
        const auto result = dist(engine_);
        return result;
    }

    void RandomGenerator::setSeed(const unsigned int seed)
    {
        std::lock_guard lock(mutex_);
        engine_.seed(seed);
    }
}