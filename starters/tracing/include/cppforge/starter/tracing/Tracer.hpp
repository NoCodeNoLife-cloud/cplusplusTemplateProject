/**
 * @file Tracer.hpp
 * @brief In-memory tracer that manages spans and supports parent-child relationships
 * @details Creates spans, stores completed spans in memory, and provides
 *          an export interface for future integration with external tracing systems.
 *
 * @par Thread Safety
 * This class is thread-safe. All operations are protected by a mutex.
 */

#pragma once
#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <cppforge/starter/tracing/Span.hpp>

namespace cppforge::starter::tracing
{
    /// @brief In-memory tracer for distributed tracing
    /// @details Manages span lifecycle and stores completed spans.
    class Tracer
    {
    public:
        /// @brief Construct a tracer with a maximum span storage capacity
        /// @param max_spans_stored Maximum number of completed spans to retain
        explicit Tracer(std::size_t max_spans_stored = 10000);

        /// @brief Start a new root span
        /// @param operation_name Name of the operation
        /// @return Shared pointer to the new span
        [[nodiscard]] std::shared_ptr<Span> startSpan(const std::string& operation_name);

        /// @brief Start a child span inheriting the trace_id from the parent
        /// @param operation_name Name of the operation
        /// @param parent The parent span
        /// @return Shared pointer to the new child span
        [[nodiscard]] std::shared_ptr<Span> startSpan(const std::string& operation_name,
                                                       const std::shared_ptr<Span>& parent);

        /// @brief Finish a span and store it
        /// @param span The span to finish
        void finishSpan(const std::shared_ptr<Span>& span);

        /// @brief Get all completed spans (for export)
        /// @return Vector of completed spans
        [[nodiscard]] std::vector<std::shared_ptr<Span>> getCompletedSpans() const;

        /// @brief Get the number of completed spans stored
        /// @return Number of stored spans
        [[nodiscard]] std::size_t completedSpanCount() const;

        /// @brief Clear all stored completed spans
        void clearCompletedSpans();

    private:
        mutable std::mutex mutex_;
        std::size_t max_spans_stored_;
        std::deque<std::shared_ptr<Span>> completed_spans_;
    };
}
