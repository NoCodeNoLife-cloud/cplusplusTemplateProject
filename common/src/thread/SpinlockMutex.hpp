/**
 * @file SpinlockMutex.hpp
 * @brief SpinlockMutex class declaration
 * @details This header defines the SpinlockMutex class that provides functionality for Threading utilities and thread pool implementation.
 */

#pragma once
#include <atomic>
#include <chrono>

namespace common::thread
{
    /// @brief A spinlock mutex implementation using atomic_flag for synchronization.
    /// This class provides a simple and efficient synchronization mechanism
    /// suitable for protecting small critical sections where the lock is held
    /// for a short duration.
    class SpinlockMutex
    {
    public:
        /// @brief Default constructor initializes the spinlock in unlocked state.
        SpinlockMutex()  = default;

        /// @brief Copy constructor is deleted to prevent copying of mutex.
        SpinlockMutex(const SpinlockMutex&) = delete;

        /// @brief Assignment operator is deleted to prevent assignment of mutex.
        SpinlockMutex& operator=(const SpinlockMutex&) = delete;

        /// @brief Move constructor is deleted to prevent moving of mutex.
        SpinlockMutex(SpinlockMutex&&) = delete;

        /// @brief Move assignment operator is deleted to prevent moving of mutex.
        SpinlockMutex& operator=(SpinlockMutex&&) = delete;

        /// @brief Locks the spinlock, blocking until the lock is acquired.
        /// Uses test_and_set in a loop with yielding to reduce CPU usage.
        void lock() ;

        /// @brief Attempts to lock the spinlock without blocking.
        /// @return true if the lock was acquired, false otherwise.
        [[nodiscard]] bool try_lock() ;

        /// @brief Attempts to lock the spinlock within the specified timeout period.
        /// @param timeout_duration Maximum time to wait for the lock
        /// @return true if the lock was acquired, false if timeout occurred
        template <typename Rep, typename Period>
        [[nodiscard]] bool try_lock_for(const std::chrono::duration<Rep, Period>& timeout_duration) ;

        /// @brief Unlocks the spinlock, allowing other threads to acquire it.
        /// Clears the atomic flag with release memory ordering.
        void unlock() ;

    private:
        std::atomic_flag flag_{};
    };
}