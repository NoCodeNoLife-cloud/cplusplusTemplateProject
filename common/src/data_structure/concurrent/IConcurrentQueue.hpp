/**
 * @file IConcurrentQueue.hpp
 * @brief Abstract interface for a concurrent (thread-safe) FIFO queue
 * @details This header defines the IConcurrentQueue interface that provides
 *          a thread-safe FIFO container contract.  Enqueue and try_dequeue
 *          operations are safe to call concurrently from any number of
 *          producer and consumer threads.
 *
 * Reference: java.util.concurrent.ConcurrentLinkedQueue (JSR 166).
 */

#pragma once

#include <cstddef>
#include <optional>

namespace common::data_structure::concurrent
{
    /// @brief Thread-safe FIFO queue interface.
    ///
    /// A concurrent queue supporting multiple producers and multiple consumers
    /// with lock-free or fine-grained locking guarantees depending on the
    /// concrete implementation.
    ///
    /// @tparam T Element type stored in the queue.
    ///
    /// @par Thread Safety
    /// All public methods are thread-safe.  Specific concurrency guarantees
    /// (lock-free vs blocking, progress guarantees) depend on the concrete
    /// implementation.  See subclass documentation for details.
    ///
    /// @par Usage Example
    /// @code
    /// class MyQueue : public IConcurrentQueue<int> { ... };
    /// MyQueue q;
    /// q.enqueue(42);
    /// if (auto val = q.try_dequeue()) {
    ///     // use *val
    /// }
    /// @endcode
    template <typename T>
    class IConcurrentQueue
    {
    public:
        virtual ~IConcurrentQueue() = default;

        // ── Modifiers ──────────────────────────────────────────────────

        /// @brief Enqueues a value at the tail of the queue.
        /// @param value The value to enqueue.
        virtual void enqueue(T value) = 0;

        /// @brief Attempts to dequeue a value from the head of the queue.
        /// @return The dequeued value if the queue was non-empty, or
        ///         std::nullopt if the queue was empty.
        [[nodiscard]] virtual auto try_dequeue() -> std::optional<T> = 0;

        // ── Capacity ───────────────────────────────────────────────────

        /// @brief Returns the approximate number of elements in the queue.
        /// @return Approximate element count.  Under concurrent operations
        ///         the returned value may momentarily lag behind the true
        ///         count.
        [[nodiscard]] virtual auto size() const -> std::size_t = 0;

        /// @brief Checks whether the queue is empty.
        /// @return true if the queue has no elements.
        [[nodiscard]] virtual auto empty() const -> bool = 0;
    };

} // namespace common::data_structure::concurrent
