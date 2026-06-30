/**
 * @file Span.hpp
 * @brief Represents a single unit of work in a distributed trace
 * @details A Span tracks an operation with a name, trace ID, span ID, tags, logs,
 *          and start/end timestamps. Spans form a tree via parent-child relationships
 *          (shared trace_id, different span_id).
 *
 * @par Thread Safety
 * Span is NOT thread-safe. Each thread should use its own span instance.
 * Do not share spans across threads.
 */

#pragma once
#include <chrono>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace cppforge::starter::tracing
{
    /// @brief Represents a single operation within a distributed trace
    class Span
    {
    public:
        /// @brief Construct a new span
        /// @param operation_name Name of the operation being traced
        /// @param trace_id Trace ID (empty string generates a new one)
        explicit Span(std::string operation_name, std::string trace_id = "");

        /// @brief Set a tag on this span
        /// @param key Tag key
        /// @param value Tag value
        void setTag(const std::string& key, const std::string& value);

        /// @brief Add a log entry to this span
        /// @param message Log message
        void log(const std::string& message);

        /// @brief Mark this span as finished (sets end time)
        void finish();

        /// @brief Get the trace ID
        /// @return The trace ID string
        [[nodiscard]] std::string traceId() const;

        /// @brief Get the span ID
        /// @return The span ID string
        [[nodiscard]] std::string spanId() const;

        /// @brief Get the operation name
        /// @return The operation name
        [[nodiscard]] const std::string& operationName() const;

        /// @brief Get the start time
        /// @return Start time point
        [[nodiscard]] std::chrono::steady_clock::time_point startTime() const;

        /// @brief Get the end time
        /// @return End time point (zero if not finished)
        [[nodiscard]] std::chrono::steady_clock::time_point endTime() const;

        /// @brief Check if the span has been finished
        /// @return true if finish() has been called
        [[nodiscard]] bool isFinished() const;

        /// @brief Get all tags
        /// @return Map of tag key-value pairs
        [[nodiscard]] const std::map<std::string, std::string>& tags() const;

        /// @brief Get all log entries
        /// @return Vector of (timestamp, message) pairs
        [[nodiscard]] const std::vector<std::pair<std::chrono::steady_clock::time_point, std::string>>& logs() const;

    private:
        std::string operation_name_;
        std::string trace_id_;
        std::string span_id_;
        std::map<std::string, std::string> tags_;
        std::vector<std::pair<std::chrono::steady_clock::time_point, std::string>> logs_;
        std::chrono::steady_clock::time_point start_time_;
        std::chrono::steady_clock::time_point end_time_;
        bool finished_ = false;

        /// @brief Generate a random hex ID
        /// @param length Number of hex characters
        /// @return Random hex string
        [[nodiscard]] static std::string generateId(std::size_t length);
    };
}