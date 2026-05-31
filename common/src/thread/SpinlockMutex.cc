/**
 * @file SpinlockMutex.cc
 * @brief SpinlockMutex class implementation
 * @details This file contains the implementation of the SpinlockMutex class methods for Threading utilities and thread pool implementation.
 */

#include "thread/SpinlockMutex.hpp"

#include <fmt/format.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <sstream>

namespace common::thread
{
    namespace
    {
        // Helper function to convert thread::id to string
        std::string getThreadIdString()
        {
            std::ostringstream oss;
            oss << std::this_thread::get_id();
            return oss.str();
        }
    }

    void SpinlockMutex::lock()
    {
        // Use exponential backoff to reduce contention
        int spin_count = 0;
        while (flag_.test_and_set(std::memory_order_acquire))
        {
            if (++spin_count > 100)
            {
                // After several attempts, yield to other threads
                std::this_thread::yield();
                spin_count = 0;
            }
            else
            {
                // On modern CPUs, pause instruction can be more efficient than yielding
#if defined(_MSC_VER) || defined(__GNUC__)
                _mm_pause(); // x86 specific pause instruction, requires <immintrin.h> indirectly
#endif
            }
        }
    }

    bool SpinlockMutex::try_lock()
    {
        // Attempt to acquire the lock without blocking
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    template <typename Rep, typename Period>
    bool SpinlockMutex::try_lock_for(const std::chrono::duration<Rep, Period>& timeout_duration)
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        auto end_time = start_time + timeout_duration;

        // First attempt without waiting
        if (try_lock())
        {
            return true;
        }

        // Loop until timeout attempting to acquire the lock
        while (std::chrono::high_resolution_clock::now() < end_time)
        {
            if (try_lock())
            {
                return true;
            }

            // Small delay to reduce CPU usage during polling
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }

        return false;
    }

    void SpinlockMutex::unlock()
    {
        flag_.clear(std::memory_order_release);
    }

    // Explicitly instantiate the template method for common duration types
    template bool SpinlockMutex::try_lock_for<>(const std::chrono::duration<int64_t, std::milli>&) ;

    template bool SpinlockMutex::try_lock_for<>(const std::chrono::duration<int64_t, std::nano>&) ;

    template bool SpinlockMutex::try_lock_for<>(const std::chrono::duration<double, std::milli>&) ;
}