/**
* @file UuidGenerator.cc
 * @brief UuidGenerator class implementation
 * @details This file contains the implementation of the UuidGenerator class methods for Random number and ID generation utilities.
 */

#include "gen/UuidGenerator.hpp"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace common::gen
{
    std::string UuidGenerator::GenerateRandomUuid() noexcept
    {
        thread_local auto generator = boost::uuids::random_generator();
        return boost::uuids::to_string(generator());
    }
}
