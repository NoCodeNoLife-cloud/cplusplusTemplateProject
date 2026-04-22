#include "src/gen/UuidGenerator.hpp"

#include <glog/logging.h>
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
        DLOG(INFO) << fmt::format("UuidGenerator GenerateRandomUuid - generated: {}", uuid_str);
        return uuid_str;
    }
}
