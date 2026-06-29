/**
 * @file SnowflakeGenerator.hpp
 * @brief Twitter-style Snowflake ID generator (64-bit unique IDs)
 * @description Generates globally unique, time-sortable 64-bit IDs following
 *          the Snowflake format: timestamp + worker ID + sequence number.
 *          IDs are monotonically increasing within the same worker process.
 *          Suitable for distributed primary key generation.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access in multi-threaded contexts.
 *
 * @par Usage Example
 * @code
 * SnowflakeGenerator gen(42); // worker ID = 42
 * uint64_t id = gen.nextId();
 * @endcode
 *
 * @par Reference
 * Twitter Engineering Blog, "Announcing Snowflake" (2010).
 */

#pragma once
#include <cstdint>
#include <mutex>

namespace cppforge::gen
{
    /// @brief Configuration options for Snowflake ID generation
    struct SnowflakeOption
    {
        static constexpr int64_t machine_id_bits_ = 5;
        static constexpr int64_t datacenter_id_bits_ = 5;
        static constexpr int64_t worker_bits_ = machine_id_bits_ + datacenter_id_bits_;
        static constexpr int64_t sequence_bits_ = 12;
        static constexpr int64_t max_sequence_ = ~(-1LL << sequence_bits_);
        static constexpr int64_t max_machine_id_ = ~(-1LL << machine_id_bits_);
        static constexpr int64_t max_datacenter_id_ = ~(-1LL << datacenter_id_bits_);

        /// @brief Twitter Snowflake epoch (2010-11-04 09:42:54.657 UTC)
        /// This is the starting timestamp from which all generated IDs are relative
        static constexpr int64_t TWEPOCH = 1288855200000LL;
    };

    /// @brief A utility class for generating unique IDs based on the Snowflake algorithm.
    /// The SnowflakeGenerator class produces 64-bit unique identifiers that combine:
    /// - A timestamp (42 bits)
    /// - A datacenter ID (5 bits)
    /// - A machine ID (5 bits)
    /// - A sequence number (12 bits)
    ///
    /// This implementation is thread-safe using a mutex to protect internal state during ID generation.
    /// Example usage:
    /// @code
    ///   cppforge::SnowflakeGenerator generator(1, 1);
    ///   int64_t id = generator.NextId();
    /// @endcode
    class SnowflakeGenerator
    {
    public:
        /// @brief Construct a SnowflakeGenerator with specified machine and datacenter IDs
        /// @param machine_id The machine ID (0-31)
        /// @param datacenter_id The datacenter ID (0-31)
        /// @throws std::invalid_argument If machine_id or datacenter_id is out of valid range
        SnowflakeGenerator(int16_t machine_id, int16_t datacenter_id);

        /// @brief Generate the next unique ID.
        /// @return The next unique ID.
        [[nodiscard]] int64_t NextId();

    private:
        /// @brief Generates the unique ID from timestamp, datacenter, machine, and sequence
        /// @param timestamp The timestamp component
        /// @param datacenter_id The datacenter ID component
        /// @param machine_id The machine ID component
        /// @param sequence The sequence number component
        /// @return Generated unique ID
        [[nodiscard]] static int64_t GenerateUniqueId(int64_t timestamp, int16_t datacenter_id, int16_t machine_id, int64_t sequence);

        /// @brief Get current timestamp in milliseconds.
        /// @return Current timestamp.
        [[nodiscard]] static int64_t GetCurrentTimestamp() ;

        /// @brief Wait until next millisecond when sequence number overflows.
        /// @param last_timestamp The last timestamp.
        /// @return The next valid timestamp.
        [[nodiscard]] static int64_t TilNextMillis(int64_t last_timestamp) ;

        int64_t last_timestamp_{-1};
        int64_t sequence_{0};
        int16_t machine_id_{0};
        int16_t datacenter_id_{0};
        mutable std::mutex mutex_{};
    };
}