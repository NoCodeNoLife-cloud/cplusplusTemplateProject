/**
 * @file Tracer.cc
 * @brief Tracer implementation
 */

#include <cppforge/starter/tracing/Tracer.hpp>
#include <glog/logging.h>

namespace cppforge::starter::tracing
{
    Tracer::Tracer(std::size_t max_spans_stored)
        : max_spans_stored_(max_spans_stored)
    {
    }

    std::shared_ptr<Span> Tracer::startSpan(const std::string& operation_name)
    {
        auto span = std::make_shared<Span>(operation_name);
        LOG(INFO) << "Started span: " << operation_name << " (trace=" << span->traceId()
                  << ", span=" << span->spanId() << ")";
        return span;
    }

    std::shared_ptr<Span> Tracer::startSpan(const std::string& operation_name,
                                             const std::shared_ptr<Span>& parent)
    {
        auto span = std::make_shared<Span>(operation_name, parent->traceId());
        span->setTag("parent_span_id", parent->spanId());
        LOG(INFO) << "Started child span: " << operation_name << " (trace=" << span->traceId()
                  << ", span=" << span->spanId() << ", parent=" << parent->spanId() << ")";
        return span;
    }

    void Tracer::finishSpan(const std::shared_ptr<Span>& span)
    {
        if (!span)
        {
            return;
        }

        span->finish();

        std::lock_guard<std::mutex> lock(mutex_);

        if (completed_spans_.size() >= max_spans_stored_)
        {
            completed_spans_.pop_front();
        }

        completed_spans_.push_back(span);
    }

    std::vector<std::shared_ptr<Span>> Tracer::getCompletedSpans() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return {completed_spans_.begin(), completed_spans_.end()};
    }

    std::size_t Tracer::completedSpanCount() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return completed_spans_.size();
    }

    void Tracer::clearCompletedSpans()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        completed_spans_.clear();
    }
}
