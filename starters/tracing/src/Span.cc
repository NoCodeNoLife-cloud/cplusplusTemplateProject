/**
 * @file Span.cc
 * @brief Span implementation
 */

#include <cppforge/starter/tracing/Span.hpp>
#include <random>
#include <sstream>
#include <iomanip>

namespace cppforge::starter::tracing
{
    Span::Span(std::string operation_name, std::string trace_id)
        : operation_name_(std::move(operation_name))
        , trace_id_(trace_id.empty() ? generateId(32) : std::move(trace_id))
        , span_id_(generateId(16))
        , start_time_(std::chrono::steady_clock::now())
        , end_time_{}
    {
    }

    void Span::setTag(const std::string& key, const std::string& value)
    {
        tags_[key] = value;
    }

    void Span::log(const std::string& message)
    {
        logs_.emplace_back(std::chrono::steady_clock::now(), message);
    }

    void Span::finish()
    {
        if (!finished_)
        {
            end_time_ = std::chrono::steady_clock::now();
            finished_ = true;
        }
    }

    std::string Span::traceId() const
    {
        return trace_id_;
    }

    std::string Span::spanId() const
    {
        return span_id_;
    }

    const std::string& Span::operationName() const
    {
        return operation_name_;
    }

    std::chrono::steady_clock::time_point Span::startTime() const
    {
        return start_time_;
    }

    std::chrono::steady_clock::time_point Span::endTime() const
    {
        return end_time_;
    }

    bool Span::isFinished() const
    {
        return finished_;
    }

    const std::map<std::string, std::string>& Span::tags() const
    {
        return tags_;
    }

    const std::vector<std::pair<std::chrono::steady_clock::time_point, std::string>>& Span::logs() const
    {
        return logs_;
    }

    std::string Span::generateId(std::size_t length)
    {
        static thread_local std::mt19937_64 gen(std::random_device{}());
        static constexpr char hex_chars[] = "0123456789abcdef";

        std::string id;
        id.reserve(length);
        for (std::size_t i = 0; i < length; ++i)
        {
            id += hex_chars[gen() % 16];
        }
        return id;
    }
}
