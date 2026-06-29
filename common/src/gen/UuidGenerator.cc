/**
 * @file UuidGenerator.cc
 * @brief UuidGenerator implementation â€?Boost.UUID random_generator wrapper
 * @details Implements UUID v4 generation via boost::uuids::random_generator.
 *          Converts the generated UUID to standard string format via
 *          boost::uuids::to_string.
 */

#include "gen/UuidGenerator.hpp"

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace cppforge::gen
{
    std::string UuidGenerator::GenerateRandomUuid() noexcept
    {
        thread_local auto generator = boost::uuids::random_generator();
        return boost::uuids::to_string(generator());
    }
}
