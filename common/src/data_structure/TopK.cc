#include "TopK.hpp"

#include <glog/logging.h>
#include <fmt/format.h>

namespace common::data_structure {
    TopK::TopK(const int32_t k) : k_(k) {
        if (k <= 0) {
            DLOG(ERROR) << fmt::format("TopK initialization failed - invalid k value: {}", k);
            throw std::invalid_argument("k must be positive");
        }
        DLOG(INFO) << fmt::format("TopK initialized with k = {}", k);
    }

    auto TopK::add(const int32_t num) -> void {
        if (minHeap_.size() < k_) {
            minHeap_.push(num);
            DLOG(INFO) << fmt::format("TopK add - number: {}, heap size: {}/{}", num, minHeap_.size(), k_);
        } else if (num > minHeap_.top()) {
            const int32_t removed = minHeap_.top();
            minHeap_.pop();
            minHeap_.push(num);
            DLOG(INFO) << fmt::format("TopK add - replaced {} with {}, heap size: {}/{}", removed, num, minHeap_.size(), k_);
        } else {
            DLOG(INFO) << fmt::format("TopK add - number {} ignored (smaller than current min: {})", num, minHeap_.top());
        }
    }

    auto TopK::getTopK() -> std::vector<int32_t> {
        DLOG(INFO) << fmt::format("TopK getTopK - retrieving top {} numbers from heap of size {}", k_, minHeap_.size());
        std::vector<int32_t> result;
        while (!minHeap_.empty()) {
            result.push_back(minHeap_.top());
            minHeap_.pop();
        }
        for (int32_t num: result) {
            minHeap_.push(num);
        }
        DLOG(INFO) << fmt::format("TopK getTopK completed - retrieved {} numbers", result.size());
        return result;
    }

    auto TopK::size() const -> size_t {
        return minHeap_.size();
    }

    auto TopK::empty() const -> bool {
        return minHeap_.empty();
    }
}
