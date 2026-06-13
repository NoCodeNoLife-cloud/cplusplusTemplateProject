/**
 * @file TopK.cc
 * @brief TopK implementation — min-heap-based stream Top-K tracking
 * @details Implements the bounded and unbounded modes of the TopK class.
 *          In bounded mode, a std::priority_queue (min-heap) of capacity K
 *          retains only the K largest elements seen.  In unbounded mode, the
 *          heap grows with every insertion.
 */

#include "TopK.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>

namespace common::data_structure
{
    TopK::TopK() : max_capacity_(0)
    {
    }

    TopK::TopK(const int32_t max_capacity) : max_capacity_(max_capacity)
    {
        if (max_capacity < 0)
        {
            throw std::invalid_argument("max_capacity must be non-negative");
        }
    }

    void TopK::add(const int32_t num)
    {
        if (max_capacity_ > 0 && static_cast<int32_t>(minHeap_.size()) >= max_capacity_)
        {
            if (num > minHeap_.top())
            {
                minHeap_.pop();
                minHeap_.push(num);
            }
        }
        else
        {
            minHeap_.push(num);
        }
    }

    std::vector<int32_t> TopK::getTopK(int32_t count, bool ascending) const
    {
        if (count < 0)
        {
            throw std::invalid_argument(
                "TopK::getTopK: count must be non-negative, got " + std::to_string(count)
            );
        }

        auto temp = minHeap_;
        const auto heap_size = static_cast<int32_t>(temp.size());
        const auto n = (count <= 0 || count > heap_size) ? heap_size : count;

        std::vector<int32_t> result;
        result.reserve(n);

        // Pop the smallest (heap_size - n) elements to waste, keeping the top n
        for (int32_t i = heap_size - n; i > 0; --i)
        {
            temp.pop();
        }

        // Extract remaining n elements (they come out in ascending order)
        while (!temp.empty())
        {
            result.push_back(temp.top());
            temp.pop();
        }

        if (!ascending)
        {
            std::reverse(result.begin(), result.end());
        }

        return result;
    }

    size_t TopK::size() const
    {
        return minHeap_.size();
    }

    bool TopK::empty() const
    {
        return minHeap_.empty();
    }
}
