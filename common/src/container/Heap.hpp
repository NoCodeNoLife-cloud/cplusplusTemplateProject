/**
 * @file Heap.hpp
 * @brief Heap class declaration
 * @details This header defines the Heap class that provides functionality for Container data structures including Queue, Stack, Heap, and UnionSet.
 */

#pragma once
#include <fmt/format.h>
#include <concepts>
#include <functional>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <utility>

namespace common::container
{
    /// @brief A heap data structure implementation.
    /// @tparam T The type of elements stored in the heap.
    /// @tparam Compare The comparison function object type that defines the heap order.
    template <std::copyable T, typename Compare = std::less<T>>
    class Heap
    {
    public:
        /// @brief Default constructor
        Heap();

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
        ~Heap();

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
        [[nodiscard]] const T& top() const;

        /// @brief Accesses the top element of the heap.
        /// @return Reference to the top element.
        /// @throws std::out_of_range If the heap is empty.
        [[nodiscard]] T& top();

        /// @brief Returns the number of elements in the heap.
        /// @return The number of elements.
        [[nodiscard]] std::size_t size() const noexcept;

        /// @brief Checks if the heap is empty.
        /// @return True if the heap is empty, false otherwise.
        [[nodiscard]] bool empty() const noexcept;

        /// @brief Clears the heap contents
        void clear() noexcept;

        /// @brief Checks if the heap is valid (maintains heap property)
        /// @return True if heap property is maintained
        [[nodiscard]] bool is_valid() const;

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
        [[nodiscard]] bool validate_heap_property() const;
    };

    template <std::copyable T, typename Compare>
    Heap<T, Compare>::Heap() = default;

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
    Heap<T, Compare>::~Heap() = default;

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::push(const T& value)
    {
        try
        {
            data_.push_back(value);
            heapify_up(data_.size() - 1);
        }
        catch (const std::bad_alloc&)
        {
            throw std::runtime_error("Heap::push: Failed to allocate memory for new element");
        }
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::push(T&& value)
    {
        try
        {
            data_.push_back(std::move(value));
            heapify_up(data_.size() - 1);
        }
        catch (const std::bad_alloc&)
        {
            throw std::runtime_error("Heap::push: Failed to allocate memory for new element");
        }
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
    const T& Heap<T, Compare>::top() const
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
    std::size_t Heap<T, Compare>::size() const noexcept
    {
        return data_.size();
    }

    template <std::copyable T, typename Compare>
    bool Heap<T, Compare>::empty() const noexcept
    {
        return data_.empty();
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::clear() noexcept
    {
        data_.clear();
    }

    template <std::copyable T, typename Compare>
    bool Heap<T, Compare>::is_valid() const
    {
        return validate_heap_property();
    }

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::heapify()
    {
        const auto size = data_.size();
        if (size <= 1) return;

        // Start from the last non-leaf node and heapify down
        for (std::int32_t i = static_cast<std::int32_t>(size) / 2 - 1; i >= 0; --i)
        {
            heapify_down(static_cast<std::size_t>(i));
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
            std::size_t largest = index;

            // Check left child
            if (left < size && compare_(data_[largest], data_[left]))
            {
                largest = left;
            }

            // Check right child - Fixed the logic error here
            if (right < size && compare_(data_[largest], data_[right]))
            {
                largest = right;
            }

            // If largest is still the current node, we're done
            if (largest == index)
            {
                break;
            }

            // Swap and continue heapifying down
            std::swap(data_[index], data_[largest]);
            index = largest;
        }
    }

    template <std::copyable T, typename Compare>
    bool Heap<T, Compare>::validate_heap_property() const
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

    template <std::copyable T, typename Compare>
    void Heap<T, Compare>::swap(Heap& other) noexcept
    {
        using std::swap;
        swap(data_, other.data_);
        swap(compare_, other.compare_);
    }
}