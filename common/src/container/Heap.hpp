#pragma once
#include <glog/logging.h>
#include <fmt/format.h>
#include <functional>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>

namespace common::container {
    /// @brief A heap data structure implementation.
    /// @tparam T The type of elements stored in the heap.
    /// @tparam Compare The comparison function object type that defines the heap order.
    template<typename T, typename Compare = std::less<T> >
    class Heap {
    public:
        /// @brief Default constructor
        Heap();

        /// @brief Constructor from iterator range
        /// @tparam Iterator Type of the iterators
        /// @param begin Start iterator
        /// @param end End iterator
        template<typename Iterator>
        Heap(Iterator begin, Iterator end);

        /// @brief Copy constructor
        Heap(const Heap &other);

        /// @brief Move constructor
        Heap(Heap &&other) noexcept;

        /// @brief Assignment operator
        Heap &operator=(const Heap &other);

        /// @brief Move assignment operator
        Heap &operator=(Heap &&other) noexcept;

        /// @brief Destructor
        ~Heap();

        /// @brief Pushes a value to the heap.
        /// @param value The value to push.
        auto push(const T &value) -> void;

        /// @brief Pushes a value to the heap.
        /// @param value The value to push.
        auto push(T &&value) -> void;

        /// @brief Constructs an element in-place and pushes it to the heap.
        /// @param args Arguments to forward to the constructor of the element.
        template<typename... Args>
        auto emplace(Args &&... args) -> void;

        /// @brief Removes the top element from the heap.
        /// @throws std::out_of_range If the heap is empty.
        auto pop() -> void;

        /// @brief Accesses the top element of the heap.
        /// @return Const reference to the top element.
        /// @throws std::out_of_range If the heap is empty.
        [[nodiscard]] auto top() const -> const T &;

        /// @brief Accesses the top element of the heap.
        /// @return Reference to the top element.
        /// @throws std::out_of_range If the heap is empty.
        [[nodiscard]] auto top() -> T &;

        /// @brief Returns the number of elements in the heap.
        /// @return The number of elements.
        [[nodiscard]] auto size() const noexcept -> std::size_t;

        /// @brief Checks if the heap is empty.
        /// @return True if the heap is empty, false otherwise.
        [[nodiscard]] auto empty() const noexcept -> bool;

        /// @brief Clears the heap contents
        auto clear() noexcept -> void;

        /// @brief Checks if the heap is valid (maintains heap property)
        /// @return True if heap property is maintained
        [[nodiscard]] auto is_valid() const -> bool;

    private:
        std::vector<T> data_{};
        Compare compare_{};

        /// @brief Heapifies the entire heap to maintain the heap property.
        auto heapify() -> void;

        /// @brief Heapifies up from the given index to maintain the heap property.
        /// @param index The index to start heapifying up from.
        auto heapify_up(std::size_t index) -> void;

        /// @brief Heapifies down from the given index to maintain the heap property.
        /// @param index The index to start heapifying down from.
        auto heapify_down(std::size_t index) -> void;

        /// @brief Validates the heap property for the entire structure
        /// @return True if heap property is maintained throughout the structure
        [[nodiscard]] auto validate_heap_property() const -> bool;
    };

    template<typename T, typename Compare>
    Heap<T, Compare>::Heap() = default;

    template<typename T, typename Compare>
    template<typename Iterator>
    Heap<T, Compare>::Heap(Iterator begin, Iterator end) : data_(begin, end) {
        heapify();
    }

    template<typename T, typename Compare>
    Heap<T, Compare>::Heap(const Heap &other) = default;

    template<typename T, typename Compare>
    Heap<T, Compare>::Heap(Heap &&other) noexcept = default;

    template<typename T, typename Compare>
    Heap<T, Compare> &Heap<T, Compare>::operator=(const Heap &other) = default;

    template<typename T, typename Compare>
    Heap<T, Compare> &Heap<T, Compare>::operator=(Heap &&other) noexcept = default;

    template<typename T, typename Compare>
    Heap<T, Compare>::~Heap() = default;

    template<typename T, typename Compare>
    auto Heap<T, Compare>::push(const T &value) -> void {
        data_.push_back(value);
        heapify_up(data_.size() - 1);
        DLOG(INFO) << fmt::format("Heap push - element added, size: {}", data_.size());
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::push(T &&value) -> void {
        data_.push_back(std::move(value));
        heapify_up(data_.size() - 1);
        DLOG(INFO) << fmt::format("Heap push (move) - element added, size: {}", data_.size());
    }

    template<typename T, typename Compare>
    template<typename... Args>
    auto Heap<T, Compare>::emplace(Args &&... args) -> void {
        data_.emplace_back(std::forward<Args>(args)...);
        heapify_up(data_.size() - 1);
        DLOG(INFO) << fmt::format("Heap emplace - element constructed in-place, size: {}", data_.size());
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::pop() -> void {
        if (empty()) {
            DLOG(ERROR) << "Heap pop failed - heap is empty";
            throw std::out_of_range("Heap is empty");
        }
        std::swap(data_[0], data_.back());
        data_.pop_back();
        if (!empty()) {
            heapify_down(0);
        }
        DLOG(INFO) << fmt::format("Heap pop - top element removed, remaining size: {}", data_.size());
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::top() const -> const T & {
        if (empty()) {
            DLOG(ERROR) << "Heap top access failed - heap is empty";
            throw std::out_of_range("Heap is empty");
        }
        return data_[0];
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::top() -> T & {
        if (empty()) {
            DLOG(ERROR) << "Heap top access failed - heap is empty";
            throw std::out_of_range("Heap is empty");
        }
        return data_[0];
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::size() const noexcept -> std::size_t {
        return data_.size();
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::empty() const noexcept -> bool {
        return data_.empty();
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::clear() noexcept -> void {
        const size_t previous_size = data_.size();
        data_.clear();
        if (previous_size > 0) {
            DLOG(INFO) << fmt::format("Heap cleared - removed {} elements", previous_size);
        }
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::is_valid() const -> bool {
        return validate_heap_property();
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::heapify() -> void {
        const auto size = data_.size();
        if (size <= 1) return;

        // Start from the last non-leaf node and heapify down
        for (std::int32_t i = static_cast<std::int32_t>(size) / 2 - 1; i >= 0; --i) {
            heapify_down(static_cast<std::size_t>(i));
        }
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::heapify_up(std::size_t index) -> void {
        while (index > 0) {
            const std::size_t parent = (index - 1) / 2;
            if (!compare_(data_[parent], data_[index])) {
                break;
            }
            std::swap(data_[index], data_[parent]);
            index = parent;
        }
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::heapify_down(std::size_t index) -> void {
        const auto size = data_.size();
        while (true) {
            const std::size_t left = 2 * index + 1;
            const std::size_t right = 2 * index + 2;
            std::size_t largest = index;

            // Check left child
            if (left < size && compare_(data_[largest], data_[left])) {
                largest = left;
            }

            // Check right child - Fixed the logic error here
            if (right < size && compare_(data_[largest], data_[right])) {
                largest = right;
            }

            // If largest is still the current node, we're done
            if (largest == index) {
                break;
            }

            // Swap and continue heapifying down
            std::swap(data_[index], data_[largest]);
            index = largest;
        }
    }

    template<typename T, typename Compare>
    auto Heap<T, Compare>::validate_heap_property() const -> bool {
        const auto size = data_.size();
        for (std::size_t i = 0; i < size; ++i) {
            const std::size_t left = 2 * i + 1;
            const std::size_t right = 2 * i + 2;

            if (left < size && compare_(data_[i], data_[left])) {
                return false;
            }
            if (right < size && compare_(data_[i], data_[right])) {
                return false;
            }
        }
        return true;
    }
}
