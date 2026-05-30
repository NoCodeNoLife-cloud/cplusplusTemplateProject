/**
 * @file TopK.hpp
 * @brief TopK class declaration
 * @details This header defines the TopK class that provides functionality for Advanced data structures including trees and skip lists.
 */

#pragma once
#include <queue>
#include <vector>
#include <cstdint>

namespace common::data_structure
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
        TopK() noexcept;

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
