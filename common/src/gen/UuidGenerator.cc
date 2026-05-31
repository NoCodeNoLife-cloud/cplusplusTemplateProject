/**
* @file UuidGenerator.cc
 * @brief UuidGenerator class implementation
 * @details This file contains the implementation of the UuidGenerator class methods for Random number and ID generation utilities.
 */

#include "gen/UuidGenerator.hpp"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

namespace common::gen
{
    // ReSharper disable once CppDFAConstantFunctionResult
    std::string UuidGenerator::GenerateRandomUuid()
    {
        static auto generator = boost::uuids::random_generator();
        const auto uuid = generator();
        const auto uuid_str = boost::uuids::to_string(uuid);
        return uuid_str;
    }
}