#include "src/gen/SnowflakeGenerator.hpp"

#include <fmt/format.h>
#include <chrono>
#include <mutex>
#include <stdexcept>
#include <cstdint>

namespace common::gen {
    SnowflakeGenerator::SnowflakeGenerator(const int16_t machine_id, const int16_t datacenter_id) {
        if (machine_id < 0 || machine_id > static_cast<int64_t>(SnowflakeOption::max_machine_id_)) {
            throw std::invalid_argument("common::SnowflakeGenerator::SnowflakeGenerator: Machine ID out of range (0-31)");
        }
        if (datacenter_id < 0 || datacenter_id > static_cast<int64_t>(SnowflakeOption::max_datacenter_id_)) {
            throw std::invalid_argument("common::SnowflakeGenerator::SnowflakeGenerator: Datacenter ID out of range (0-31)");
        }
        machine_id_ = machine_id;
        datacenter_id_ = datacenter_id;
    }

    auto SnowflakeGenerator::NextId() -> int64_t {
        std::lock_guard lock(mutex_);
        int64_t timestamp = GetCurrentTimestamp();

        if (timestamp < last_timestamp_) {
            do {
                timestamp = GetCurrentTimestamp();
            } while (timestamp < last_timestamp_);
        }

        UpdateSequenceAndTimestamp(timestamp, last_timestamp_);

        last_timestamp_ = timestamp;

        const auto uniqueId = GenerateUniqueId(timestamp, datacenter_id_, machine_id_, sequence_);
        return uniqueId;
    }

    auto SnowflakeGenerator::GetCurrentTimestamp() noexcept -> int64_t {
        const auto now = std::chrono::system_clock::now();
        const auto duration = now.time_since_epoch();
        const int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        constexpr int64_t start_time = 1288855200000LL;
        return timestamp - start_time;
    }

    auto SnowflakeGenerator::TilNextMillis(const int64_t last_timestamp) noexcept -> int64_t {
        int64_t timestamp = GetCurrentTimestamp();
        while (timestamp <= last_timestamp) {
            timestamp = GetCurrentTimestamp();
        }
        return timestamp;
    }

    auto SnowflakeGenerator::UpdateSequenceAndTimestamp(int64_t &timestamp, const int64_t last_timestamp) -> void {
        if (timestamp == last_timestamp) {
            sequence_ = sequence_ + 1 & static_cast<int64_t>(SnowflakeOption::max_sequence_);
            if (sequence_ == 0) {
                timestamp = TilNextMillis(last_timestamp);
            }
        } else {
            sequence_ = 0;
        }
    }

    auto SnowflakeGenerator::GenerateUniqueId(const int64_t timestamp, const int16_t datacenter_id, const int16_t machine_id, const int64_t sequence) -> int64_t {
        return timestamp << (static_cast<int64_t>(SnowflakeOption::machine_bits_) + static_cast<int64_t>(SnowflakeOption::sequence_bits_)) | static_cast<int64_t>(datacenter_id << 5 | machine_id) << static_cast<int64_t>(SnowflakeOption::sequence_bits_) | sequence;
    }
}
