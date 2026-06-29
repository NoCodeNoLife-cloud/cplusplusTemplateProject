/**
 * @file TopK.hpp
 * @brief Min-heap-based Top-K tracking for integer streams
 * @details Maintains the top K largest integers from an input stream using a
 *          fixed-capacity min-heap.  When the heap is full, each new element
 *          is compared against the smallest element in the heap; if larger,
 *          the smallest is evicted.  Supports bounded (fixed K) and unbounded
 *          modes, and can output results in ascending or descending order.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - add:   O(log K) when at capacity; O(1) otherwise
 * - query: O(K log K) for sorted output
 *
 * @par Usage Example
 * @code
 * TopK topK(3);
 * topK.add(10); topK.add(5); topK.add(20); topK.add(1);
 * auto result = topK.getTopK(); // {5, 10, 20}
 * @endcode
 */

#pragma once
#include <cstdint>
#include <queue>
#include <stdexcept>
#include <vector>

namespace cppforge::data_structure
{
    /// @brief A class to maintain the top K the largest numbers from a stream of integers.
    /// The TopK class uses a min-heap to efficiently track the top K the largest numbers.
    /// When a new number is added, it is compared with the smallest number in the heap.
    /// If the new number is larger, the smallest number is removed and the new number is added.
    /// This ensures that the heap always contains the top K the largest numbers seen so far.
    class TopK final
    {
    public:
        /// @brief Default constructor (unbounded heap, grows as elements are added).
        TopK() ;

        /// @brief Constructs a TopK with a maximum capacity.
        /// @param max_capacity Maximum number of elements to maintain in the heap.
        ///                     If 0, the heap is unbounded.
        explicit TopK(int32_t max_capacity);

        /// @brief Add a number to the TopK.
        /// @param num The number to add.
        void add(int32_t num);

        /// @brief Get the top k numbers (non-destructive, read-only operation).
        /// @param count Number of elements to return (must be <= internal heap size).
        ///             If 0 or greater than heap size, returns all elements.
        /// @param ascending If true, returns in ascending order (smallest to largest).
        ///                  If false, returns in descending order (largest to smallest).
        /// @return The top k numbers in the specified order.
        /// @throws std::invalid_argument If count is negative
        /// @note This method does not modify the internal heap state.
        [[nodiscard]] std::vector<int32_t> getTopK(int32_t count = 0, bool ascending = true) const;

        /// @brief Get the current size of the heap.
        /// @return The number of elements in the heap.
        [[nodiscard]] size_t size() const;

        /// @brief Check if the heap is empty.
        /// @return True if the heap is empty, false otherwise.
        [[nodiscard]] bool empty() const;

    private:
        int32_t max_capacity_; // 0 means unbounded
        std::priority_queue<int32_t, std::vector<int32_t>, std::greater<>> minHeap_;
    };
}