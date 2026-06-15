/**
 * @file IConcurrentStack.hpp
 * @brief Abstract interface for a concurrent (thread-safe) LIFO stack
 * @details This header defines the IConcurrentStack interface that provides
 *          a thread-safe LIFO container contract.  Push and try_pop operations
 *          are safe to call concurrently from any number of producer and
 *          consumer threads.
 *
 * Reference: java.util.concurrent.ConcurrentLinkedDeque (JSR 166).
 */

#pragma once

#include <cstddef>
#include <optional>

namespace common::data_structure::concurrent
{
    /// @brief Thread-safe LIFO stack interface.
    ///
    /// A concurrent stack supporting multiple producers and multiple consumers
    /// with lock-free or fine-grained locking guarantees depending on the
    /// concrete implementation.
    ///
    /// @tparam T Element type stored in the stack.
    ///
    /// @par Thread Safety
    /// All public methods are thread-safe.  Specific concurrency guarantees
    /// (lock-free vs blocking, progress guarantees) depend on the concrete
    /// implementation.  See subclass documentation for details.
    ///
    /// @par Usage Example
    /// @code
    /// class MyStack : public IConcurrentStack<int> { ... };
    /// MyStack s;
    /// s.push(42);
    /// if (auto val = s.try_pop()) {
    ///     // use *val
    /// }
    /// @endcode
    template <typename T>
    class IConcurrentStack
    {
    public:
        virtual ~IConcurrentStack() = default;

        // ── Modifiers ──────────────────────────────────────────────────

        /// @brief Pushes a value onto the top of the stack.
        /// @param value The value to push.
        virtual void push(T value) = 0;

        /// @brief Attempts to pop a value from the top of the stack.
        /// @return The popped value if the stack was non-empty, or
        ///         std::nullopt if the stack was empty.
        [[nodiscard]] virtual auto try_pop() -> std::optional<T> = 0;

        // ── Capacity ───────────────────────────────────────────────────

        /// @brief Returns the approximate number of elements in the stack.
        /// @return Approximate element count.  Under concurrent operations
        ///         the returned value may momentarily lag behind the true
        ///         count.
        [[nodiscard]] virtual auto size() const -> std::size_t = 0;

        /// @brief Checks whether the stack is empty.
        /// @return true if the stack has no elements.
        [[nodiscard]] virtual auto empty() const -> bool = 0;
    };

} // namespace common::data_structure::concurrent
