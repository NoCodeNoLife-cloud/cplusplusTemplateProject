/**
 * @file ConcurrentQueue.hpp
 * @brief ConcurrentQueue class declaration
 * @details This header defines the ConcurrentQueue class -- a thread-safe,
 *          multiple-producer multiple-consumer (MPMC) bounded queue using
 *          mutex and condition variables for synchronization.
 */

#pragma once
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <limits>
#include <mutex>
#include <queue>
#include <utility>
#include <stdexcept>

namespace common::data_structure
{
    /// @brief A thread-safe concurrent queue implementation
    /// @details Provides both blocking and non-blocking push/pop operations.
    ///          Supports bounded mode (fixed capacity) and unbounded mode.
    ///          Multiple threads can safely push and pop concurrently.
    /// @tparam T The type of elements stored in the queue
    template <typename T>
    class ConcurrentQueue
    {
    public:
        using size_type = std::size_t;

        /// @brief Constructs a concurrent queue with the given maximum capacity
        /// @param max_size Maximum number of elements (default: unbounded)
        explicit ConcurrentQueue(size_type max_size = std::numeric_limits<size_type>::max())
            : max_size_(max_size)
        {
        }

        ConcurrentQueue(const ConcurrentQueue&) = delete;
        ConcurrentQueue& operator=(const ConcurrentQueue&) = delete;
        ConcurrentQueue(ConcurrentQueue&&) = delete;
        ConcurrentQueue& operator=(ConcurrentQueue&&) = delete;

        // -- Blocking operations --

        /// @brief Blocking push -- waits until space is available
        /// @param value The value to add
        void push(const T& value)
        {
            std::unique_lock lock(mutex_);
            not_full_.wait(lock, [this] { return queue_.size() < max_size_; });
            queue_.push(value);
            lock.unlock();
            not_empty_.notify_one();
        }

        /// @brief Blocking push with move semantics
        /// @param value The value to move into the queue
        void push(T&& value)
        {
            std::unique_lock lock(mutex_);
            not_full_.wait(lock, [this] { return queue_.size() < max_size_; });
            queue_.push(std::move(value));
            lock.unlock();
            not_empty_.notify_one();
        }

        /// @brief Construct an element in-place at the back (blocking)
        /// @tparam Args The types of arguments to forward to T's constructor
        /// @param args The arguments to forward
        template <typename... Args>
        void emplace(Args&&... args)
        {
            std::unique_lock lock(mutex_);
            not_full_.wait(lock, [this] { return queue_.size() < max_size_; });
            queue_.emplace(std::forward<Args>(args)...);
            lock.unlock();
            not_empty_.notify_one();
        }

        /// @brief Blocking pop -- waits until an element is available
        /// @return The front element (moved out of the queue)
        T pop()
        {
            std::unique_lock lock(mutex_);
            not_empty_.wait(lock, [this] { return !queue_.empty(); });
            T value = std::move(queue_.front());
            queue_.pop();
            lock.unlock();
            not_full_.notify_one();
            return value;
        }

        /// @brief Blocking pop with timeout
        /// @tparam Rep The type of the duration's tick
        /// @tparam Period The duration's period
        /// @param value Reference to store the popped element
        /// @param timeout Maximum time to wait
        /// @return true if an element was popped, false on timeout
        template <typename Rep, typename Period>
        bool try_pop_for(T& value, const std::chrono::duration<Rep, Period>& timeout)
        {
            std::unique_lock lock(mutex_);
            if (!not_empty_.wait_for(lock, timeout, [this] { return !queue_.empty(); }))
            {
                return false;
            }
            value = std::move(queue_.front());
            queue_.pop();
            lock.unlock();
            not_full_.notify_one();
            return true;
        }

        // -- Non-blocking operations --

        /// @brief Non-blocking push attempt
        /// @param value The value to add
        /// @return true if the element was added, false if the queue is full
        bool try_push(const T& value)
        {
            std::lock_guard lock(mutex_);
            if (queue_.size() >= max_size_)
            {
                return false;
            }
            queue_.push(value);
            not_empty_.notify_one();
            return true;
        }

        /// @brief Non-blocking push attempt with move semantics
        /// @param value The value to move into the queue
        /// @return true if the element was added, false if the queue is full
        bool try_push(T&& value)
        {
            std::lock_guard lock(mutex_);
            if (queue_.size() >= max_size_)
            {
                return false;
            }
            queue_.push(std::move(value));
            not_empty_.notify_one();
            return true;
        }

        /// @brief Construct an element in-place at the back (non-blocking)
        /// @tparam Args The types of arguments to forward
        /// @param args The arguments to forward
        /// @return true if the element was added, false if the queue is full
        template <typename... Args>
        bool try_emplace(Args&&... args)
        {
            std::lock_guard lock(mutex_);
            if (queue_.size() >= max_size_)
            {
                return false;
            }
            queue_.emplace(std::forward<Args>(args)...);
            not_empty_.notify_one();
            return true;
        }

        /// @brief Non-blocking pop attempt
        /// @param value Reference to store the popped element
        /// @return true if an element was popped, false if the queue is empty
        bool try_pop(T& value)
        {
            std::lock_guard lock(mutex_);
            if (queue_.empty())
            {
                return false;
            }
            value = std::move(queue_.front());
            queue_.pop();
            not_full_.notify_one();
            return true;
        }

        // -- Capacity and status --

        /// @brief Get the approximate number of elements in the queue
        /// @return The number of elements
        [[nodiscard]] size_type size() const
        {
            std::lock_guard lock(mutex_);
            return queue_.size();
        }

        /// @brief Check if the queue is empty
        /// @return true if the queue has no elements
        [[nodiscard]] bool empty() const
        {
            std::lock_guard lock(mutex_);
            return queue_.empty();
        }

        /// @brief Get the maximum capacity of the queue
        /// @return The maximum number of elements
        [[nodiscard]] size_type capacity() const noexcept
        {
            return max_size_;
        }

        /// @brief Remove all elements from the queue and wake all waiting pushers
        void clear()
        {
            std::lock_guard lock(mutex_);
            queue_ = std::queue<T>();
            not_full_.notify_all();
        }

    private:
        std::queue<T> queue_;
        mutable std::mutex mutex_;
        std::condition_variable not_empty_;
        std::condition_variable not_full_;
        size_type max_size_;
    };
}
