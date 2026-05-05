#include "TopK.hpp"

#include <algorithm>
#include <fmt/format.h>

namespace common::data_structure {
TopK::TopK() noexcept : max_capacity_(0) {
}

TopK::TopK(const int32_t max_capacity) : max_capacity_(max_capacity) {
    if (max_capacity < 0) {
        throw std::invalid_argument("max_capacity must be non-negative");
    }
}

auto TopK::add(const int32_t num) -> void {
    // If capacity is bounded and heap is full
    if (max_capacity_ > 0 && static_cast<int32_t>(minHeap_.size()) >= max_capacity_) {
        // Only add if the new number is larger than the smallest in heap
        if (num > minHeap_.top()) {
            minHeap_.pop();
            minHeap_.push(num);
        }
    } else {
        // Heap is not full or unbounded
        minHeap_.push(num);
    }
}

auto TopK::getTopK(int32_t count, bool ascending) const -> std::vector<int32_t> {
    // Create a copy of the heap to avoid modifying the original
    auto temp_heap = minHeap_;

    // Extract all elements from the heap (min-heap gives ascending order)
    std::vector<int32_t> all_elements;
    all_elements.reserve(temp_heap.size());

    while (!temp_heap.empty()) {
        all_elements.push_back(temp_heap.top());
        temp_heap.pop();
    }

    // Determine how many elements to return
    const auto heap_size = static_cast<int32_t>(all_elements.size());
    const auto elements_to_return = (count <= 0 || count > heap_size) ? heap_size : count;

    // Get the largest 'elements_to_return' elements (last N elements from ascending list)
    std::vector<int32_t> result(
        all_elements.end() - elements_to_return,
        all_elements.end()
        );

    // If descending order is requested, reverse the result
    if (!ascending) {
        std::reverse(result.begin(), result.end());
    }

    return result;
}

auto TopK::size() const -> size_t {
    return minHeap_.size();
}

auto TopK::empty() const -> bool {
    return minHeap_.empty();
}
}