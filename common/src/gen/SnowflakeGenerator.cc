/**
 * @file SnowflakeGenerator.cc
 * @brief SnowflakeGenerator class implementation
 * @details This file contains the implementation of the SnowflakeGenerator class methods for Random number and ID generation utilities.
 */

#include "gen/SnowflakeGenerator.hpp"

#include <fmt/format.h>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <glog/logging.h>

namespace common::gen
{
    SnowflakeGenerator::SnowflakeGenerator(int16_t machine_id, int16_t datacenter_id)
    {
        if (machine_id < 0 || machine_id > SnowflakeOption::max_machine_id_)
        {
            DLOG(WARNING) << fmt::format("SnowflakeGenerator: Machine ID {} out of range (0-31)", machine_id);
            throw std::invalid_argument("common::SnowflakeGenerator::SnowflakeGenerator: Machine ID out of range (0-31)");
        }
        if (datacenter_id < 0 || datacenter_id > SnowflakeOption::max_datacenter_id_)
        {
            DLOG(WARNING) << fmt::format("SnowflakeGenerator: Datacenter ID {} out of range (0-31)", datacenter_id);
            throw std::invalid_argument("common::SnowflakeGenerator::SnowflakeGenerator: Datacenter ID out of range (0-31)");
        }
        machine_id_ = machine_id;
        datacenter_id_ = datacenter_id;
    }

    int64_t SnowflakeGenerator::NextId()
    {
        std::lock_guard lock(mutex_);
        int64_t timestamp = GetCurrentTimestamp();

        if (timestamp < last_timestamp_)
        {
            do
            {
                std::this_thread::yield();
                timestamp = GetCurrentTimestamp();
            }
            while (timestamp < last_timestamp_);
        }

        if (timestamp == last_timestamp_)
        {
            sequence_ = (sequence_ + 1) & SnowflakeOption::max_sequence_;
            if (sequence_ == 0)
            {
                timestamp = TilNextMillis(last_timestamp_);
            }
        }
        else
        {
            sequence_ = 0;
        }

        last_timestamp_ = timestamp;

        const auto uniqueId = GenerateUniqueId(timestamp, datacenter_id_, machine_id_, sequence_);
        return uniqueId;
    }

    int64_t SnowflakeGenerator::GenerateUniqueId(const int64_t timestamp, const int16_t datacenter_id, const int16_t machine_id, const int64_t sequence)
    {
        constexpr int64_t kShift = SnowflakeOption::machine_bits_ + SnowflakeOption::sequence_bits_;
        const int64_t nodeId = (static_cast<int64_t>(datacenter_id) << 5) | static_cast<int64_t>(machine_id);
        return (timestamp << kShift) | (nodeId << SnowflakeOption::sequence_bits_) | sequence;
    }

    int64_t SnowflakeGenerator::GetCurrentTimestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const auto duration = now.time_since_epoch();
        const int64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        return timestamp - SnowflakeOption::TWEPOCH;
    }

    int64_t SnowflakeGenerator::TilNextMillis(const int64_t last_timestamp)
    {
        int64_t timestamp = GetCurrentTimestamp();
        while (timestamp <= last_timestamp)
        {
            std::this_thread::yield();
            timestamp = GetCurrentTimestamp();
        }
        return timestamp;
    }
}