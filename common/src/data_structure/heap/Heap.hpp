/**
 * @file Heap.hpp
 * @brief Generic binary heap (max-heap by default) with O(log n) push/pop
 * @details A binary heap implementation backed by a contiguous std::vector.
 *          The heap order is determined by the Compare template parameter
 *          (default std::less<T> yields a max-heap; use std::greater<T> for
 *          a min-heap).  Construction from an iterator range runs in O(n)
 *          via Floyd's heapify algorithm.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Complexity
 * - push:          O(log n) amortised
 * - pop:           O(log n)
 * - top:           O(1)
 * - heapify range: O(n)
 *
 * @par Usage Example
 * @code
 * Heap<int> maxHeap;
 * maxHeap.push(10);
 * maxHeap.push(5);
 * maxHeap.push(20);
 * assert(maxHeap.top() == 20);  // max-heap default
 * @endcode
 */

#pragma once
#include <algorithm>
#include <concepts>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cppforge::data_structure
{
    /// @brief A generic binary heap with configurable ordering.
    ///
    /// @tparam T       Element type (must satisfy std::copyable).
    /// @tparam Compare Comparison functor defining the heap order.
    ///                 std::less<T>  â†?max-heap (root is largest).
    ///                 std::greater<T> â†?min-heap (root is smallest).
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe.  External synchronisation is required
    /// for concurrent reads and writes.
    ///
    /// @par Invariants
    /// - For any node at index i, !compare_(data_[parent(i)], data_[i])
    /// - The heap is stored in a contiguous vector for cache locality.
    template <std::copyable T, typename Compare = std::less<T>>
    class Heap
    {
    public:
        /// @brief Default constructor
        Heap() noexcept;

        /// @brief Constructor from iterator range
        /// @tparam Iterator Type of the iterators
        /// @param begin Start iterator
        /// @param end End iterator
        template <std::input_iterator Iterator>
        Heap(Iterator begin, Iterator end);

        /// @brief Copy constructor
        Heap(const Heap& other);

        /// @brief Move constructor
        Heap(Heap&& other) noexcept;

        /// @brief Assignment operator
        Heap& operator=(const Heap& other);

        /// @brief Move assignment operator
        Heap& operator=(Heap&& other) noexcept;

        /// @brief Destructor
        ~Heap() noexcept;

        /// @brief Pushes a value to the heap.
        /// @param value The value to push.
        /// @throws std::bad_alloc If memory allocation fails
        void push(const T& value);

        /// @brief Pushes a value to the heap.
        /// @param value The value to push.
        /// @throws std::bad_alloc If memory allocation fails
        void push(T&& value);

        /// @brief Constructs an element in-place and pushes it to the heap.
        /// @param args Arguments to forward to the constructor of the element.
        template <typename... Args>
        void emplace(Args&&... args);

        /// @brief Removes the top element from the heap.
        /// @throws std::out_of_range If the heap is empty.
        void pop();

        /// @brief Accesses the top element of the heap.
        /// @return Const reference to the top element.
        /// @throws std::out_of_range If the heap is empty.
        [[nodiscard]] constexpr const T& top() const;

        /// @brief Accesses the top element of the heap.
        /// @return Reference to the top element.
        /// @throws std::out_of_range If the heap is empty.
        [[nodiscard]] T& top();

        /// @brief Returns the number of elements in the heap.
        /// @return The number of elements.
        [[nodiscard]] constexpr std::size_t size() const noexcept;

        /// @brief Checks if the heap is empty.
        /// @return True if the heap is empty, false otherwise.
        [[nodiscard]] constexpr bool empty() const noexcept;

        /// @brief Clears the heap contents
        void clear() noexcept;

        /// @brief Checks if the heap is valid (maintains heap property)
        /// @return True if heap property is maintained
        [[nodiscard]] constexpr bool is_valid() const noexcept;

        /// @brief Swaps the contents of this heap with another heap.
        /// @param other The heap to swap with.
        void swap(Heap& other) noexcept;

    private:
        std::vector<T> data_{};
        Compare compare_{};

        /// @brief Heapifies the entire heap to maintain the heap property.
        void heapify();

        /// @brief Heapifies up from the given index to maintain the heap property.
        /// @param index The index to start heapifying up from.
        void heapify_up(std::size_t index);

        /// @brief Heapifies down from the given index to maintain the heap property.
        /// @param index The index to start heapifying down from.
        void heapify_down(std::size_t index);

        /// @brief Validates the heap property for the entire structure
        /// @return True if heap property is maintained throughout the structure
        [[nodiscard]] constexpr bool validate_heap_property() const noexcept;
    };

    template <std::copyable T, typename Compare>
    Heap<T, Compare>::Heap() noexcept = default;

    template <std::copyable T, typename Compare>
    template <std::input_iterator Iterator>
    Heap<T, Compare>::Heap(Iterator begin, Iterator end) : data_(begin, end)
    {
        heapify();
    }

    template <std::copyable T, typename Compare>
    Heap<T, Compare>::Heap(const Heap& other) = default;

    template <std::copyable T, typename Compare>
    Heap<T, Compare>::Heap(Heap&& other) noexcept = default;

    template <std::copyable T, typename Compare>
    Heap<T, Compare>& Heap<T, Compare>::operator=(const Heap& other) = default;

    template <std::copyable T, typename Compare>
    Heap<T, Compare>& Heap<T, Compare>::operator=(Heap&& other) noexcept = default;

    template <std::copyable T, typename Compare>
    Heap<T, Compare>::~Heap() noexcept = default;

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::push(const T& value)
    {
        data_.push_back(value);
        heapify_up(data_.size() - 1);
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::push(T&& value)
    {
        data_.push_back(std::move(value));
        heapify_up(data_.size() - 1);
    }

    template <std::copyable T, typename Compare>
    template <typename... Args>
    void Heap<T, Compare>::emplace(Args&&... args)
    {
        data_.emplace_back(std::forward<Args>(args)...);
        heapify_up(data_.size() - 1);
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::pop()
    {
        if (empty())
        {
            throw std::out_of_range("Heap is empty");
        }
        std::swap(data_[0], data_.back());
        data_.pop_back();
        if (!empty())
        {
            heapify_down(0);
        }
    }

    template <std::copyable T, typename Compare>
    constexpr const T& Heap<T, Compare>::top() const
    {
        if (empty())
        {
            throw std::out_of_range("Heap is empty");
        }
        return data_[0];
    }

    template <std::copyable T, typename Compare>
    T& Heap<T, Compare>::top()
    {
        if (empty())
        {
            throw std::out_of_range("Heap is empty");
        }
        return data_[0];
    }

    template <std::copyable T, typename Compare>
    constexpr std::size_t Heap<T, Compare>::size() const noexcept
    {
        return data_.size();
    }

    template <std::copyable T, typename Compare>
    constexpr bool Heap<T, Compare>::empty() const noexcept
    {
        return data_.empty();
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::clear() noexcept
    {
        data_.clear();
    }

    template <std::copyable T, typename Compare>
    constexpr bool Heap<T, Compare>::is_valid() const noexcept
    {
        return validate_heap_property();
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::swap(Heap& other) noexcept
    {
        using std::swap;
        swap(data_, other.data_);
        swap(compare_, other.compare_);
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::heapify()
    {
        const auto size = data_.size();
        if (size <= 1) return;

        for (auto i = size / 2; i > 0; --i)
        {
            heapify_down(i - 1);
        }
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::heapify_up(std::size_t index)
    {
        while (index > 0)
        {
            const std::size_t parent = (index - 1) / 2;
            if (!compare_(data_[parent], data_[index]))
            {
                break;
            }
            std::swap(data_[index], data_[parent]);
            index = parent;
        }
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::heapify_down(std::size_t index)
    {
        const auto size = data_.size();
        while (true)
        {
            const std::size_t left = 2 * index + 1;
            const std::size_t right = 2 * index + 2;
            std::size_t target = index;

            if (left < size && compare_(data_[target], data_[left]))
            {
                target = left;
            }

            if (right < size && compare_(data_[target], data_[right]))
            {
                target = right;
            }

            if (target == index)
            {
                break;
            }

            std::swap(data_[index], data_[target]);
            index = target;
        }
    }

    template <std::copyable T, typename Compare>
    constexpr bool Heap<T, Compare>::validate_heap_property() const noexcept
    {
        const auto size = data_.size();
        for (std::size_t i = 0; i < size; ++i)
        {
            const std::size_t left = 2 * i + 1;
            const std::size_t right = 2 * i + 2;

            if (left < size && compare_(data_[i], data_[left]))
            {
                return false;
            }
            if (right < size && compare_(data_[i], data_[right]))
            {
                return false;
            }
        }
        return true;
    }
}