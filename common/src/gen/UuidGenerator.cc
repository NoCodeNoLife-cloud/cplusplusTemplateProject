#include "src/gen/UuidGenerator.hpp"

#include <fmt/format.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

namespace common::gen {
    // ReSharper disable once CppDFAConstantFunctionResult
    auto UuidGenerator::GenerateRandomUuid() noexcept -> std::string {
        static auto generator = boost::uuids::random_generator();
        const auto uuid = generator();
        const auto uuid_str = boost::uuids::to_string(uuid);
        return uuid_str;
    }
}
