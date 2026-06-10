/**
 * @file RingBuffer.hpp
 * @brief RingBuffer class declaration
 * @details This header defines the RingBuffer class — a fixed-capacity circular buffer
 *          that overwrites the oldest element when full, with O(1) push/pop on both ends.
 */

#pragma once
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace common::data_structure
{
    /// @brief A fixed-capacity circular buffer (ring buffer)
    ///        When the buffer is full, pushing a new element overwrites the oldest
    /// @tparam T The type of elements stored in the buffer
    template <std::movable T>
    class RingBuffer
    {
    public:
        using size_type = std::size_t;

        /// @brief Constructs a ring buffer with the given capacity
        /// @param capacity Maximum number of elements (must be greater than 0)
        explicit RingBuffer(size_type capacity)
            : buffer_(capacity + 1)
            , capacity_(capacity + 1)
        {
            if (capacity == 0)
            {
                throw std::invalid_argument("RingBuffer capacity must be > 0");
            }
        }

        /// @brief Copy constructor creates a deep copy of another ring buffer
        /// @param other The ring buffer to copy from
        RingBuffer(const RingBuffer& other)
            : buffer_(other.buffer_)
            , head_(other.head_)
            , tail_(other.tail_)
            , capacity_(other.capacity_)
        {
        }

        /// @brief Move constructor transfers ownership from another ring buffer
        /// @param other The ring buffer to move from
        RingBuffer(RingBuffer&& other) noexcept
            : buffer_(std::move(other.buffer_))
            , head_(other.head_)
            , tail_(other.tail_)
            , capacity_(other.capacity_)
        {
            other.head_ = 0;
            other.tail_ = 0;
            other.capacity_ = 0;
        }

        /// @brief Copy assignment operator
        /// @param other The ring buffer to copy from
        /// @return Reference to this ring buffer
        RingBuffer& operator=(const RingBuffer& other)
        {
            if (this != &other)
            {
                buffer_ = other.buffer_;
                head_ = other.head_;
                tail_ = other.tail_;
                capacity_ = other.capacity_;
            }
            return *this;
        }

        /// @brief Move assignment operator
        /// @param other The ring buffer to move from
        /// @return Reference to this ring buffer
        RingBuffer& operator=(RingBuffer&& other) noexcept
        {
            if (this != &other)
            {
                buffer_ = std::move(other.buffer_);
                head_ = other.head_;
                tail_ = other.tail_;
                capacity_ = other.capacity_;
                other.head_ = 0;
                other.tail_ = 0;
                other.capacity_ = 0;
            }
            return *this;
        }

        /// @brief Destructor
        ~RingBuffer() noexcept = default;

        // -- Element access --

        /// @brief Access the first element
        /// @return Reference to the first element
        /// @throws std::out_of_range If the ring buffer is empty
        [[nodiscard]] T& front()
        {
            if (empty()) throw std::out_of_range("RingBuffer is empty");
            return buffer_[head_];
        }

        /// @brief Access the first element (const)
        /// @return Const reference to the first element
        /// @throws std::out_of_range If the ring buffer is empty
        [[nodiscard]] const T& front() const
        {
            if (empty()) throw std::out_of_range("RingBuffer is empty");
            return buffer_[head_];
        }

        /// @brief Access the last element
        /// @return Reference to the last element
        /// @throws std::out_of_range If the ring buffer is empty
        [[nodiscard]] T& back()
        {
            if (empty()) throw std::out_of_range("RingBuffer is empty");
            return buffer_[prev(tail_)];
        }

        /// @brief Access the last element (const)
        /// @return Const reference to the last element
        /// @throws std::out_of_range If the ring buffer is empty
        [[nodiscard]] const T& back() const
        {
            if (empty()) throw std::out_of_range("RingBuffer is empty");
            return buffer_[prev(tail_)];
        }

        /// @brief Access element by index (0-based)
        /// @param index Position of the element
        /// @return Reference to the element at the given index
        /// @throws std::out_of_range If index is out of bounds
        [[nodiscard]] T& at(size_type index)
        {
            if (index >= size())
            {
                throw std::out_of_range("RingBuffer index out of range");
            }
            return buffer_[(head_ + index) % capacity_];
        }

        /// @brief Access element by index (const)
        /// @param index Position of the element
        /// @return Const reference to the element at the given index
        /// @throws std::out_of_range If index is out of bounds
        [[nodiscard]] const T& at(size_type index) const
        {
            if (index >= size())
            {
                throw std::out_of_range("RingBuffer index out of range");
            }
            return buffer_[(head_ + index) % capacity_];
        }

        /// @brief Access element by index without bounds checking
        /// @param index Position of the element
        /// @return Reference to the element at the given index
        [[nodiscard]] T& operator[](size_type index)
        {
            return buffer_[(head_ + index) % capacity_];
        }

        /// @brief Access element by index without bounds checking (const)
        /// @param index Position of the element
        /// @return Const reference to the element at the given index
        [[nodiscard]] const T& operator[](size_type index) const
        {
            return buffer_[(head_ + index) % capacity_];
        }

        // -- Capacity --

        /// @brief Get the number of elements stored in the ring buffer
        /// @return The number of elements
        [[nodiscard]] size_type size() const noexcept
        {
            return (tail_ + capacity_ - head_) % capacity_;
        }

        /// @brief Get the maximum capacity of the ring buffer
        /// @return The maximum number of elements
        [[nodiscard]] size_type capacity() const noexcept
        {
            return capacity_ - 1;
        }

        /// @brief Check if the ring buffer is empty
        /// @return True if the ring buffer is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept
        {
            return head_ == tail_;
        }

        /// @brief Check if the ring buffer is full
        /// @return True if the ring buffer is full, false otherwise
        [[nodiscard]] bool full() const noexcept
        {
            return next(tail_) == head_;
        }

        // -- Modifiers --

        /// @brief Add an element to the back of the ring buffer (overwrites front if full)
        /// @param value The value to add
        void push_back(const T& value)
        {
            buffer_[tail_] = value;
            tail_ = next(tail_);
            if (tail_ == head_)
            {
                head_ = next(head_);
            }
        }

        /// @brief Add an element to the back of the ring buffer using move semantics (overwrites front if full)
        /// @param value The value to move into the ring buffer
        void push_back(T&& value)
        {
            buffer_[tail_] = std::move(value);
            tail_ = next(tail_);
            if (tail_ == head_)
            {
                head_ = next(head_);
            }
        }

        /// @brief Add an element to the front of the ring buffer (overwrites back if full)
        /// @param value The value to add
        void push_front(const T& value)
        {
            head_ = prev(head_);
            buffer_[head_] = value;
            if (head_ == tail_)
            {
                tail_ = prev(tail_);
            }
        }

        /// @brief Add an element to the front of the ring buffer using move semantics (overwrites back if full)
        /// @param value The value to move into the ring buffer
        void push_front(T&& value)
        {
            head_ = prev(head_);
            buffer_[head_] = std::move(value);
            if (head_ == tail_)
            {
                tail_ = prev(tail_);
            }
        }

        /// @brief Remove the first element from the ring buffer
        /// @throws std::out_of_range If the ring buffer is empty
        void pop_front()
        {
            if (empty()) throw std::out_of_range("RingBuffer is empty");
            head_ = next(head_);
        }

        /// @brief Remove the last element from the ring buffer
        /// @throws std::out_of_range If the ring buffer is empty
        void pop_back()
        {
            if (empty()) throw std::out_of_range("RingBuffer is empty");
            tail_ = prev(tail_);
        }

        /// @brief Remove all elements from the ring buffer
        void clear() noexcept
        {
            head_ = 0;
            tail_ = 0;
        }

        /// @brief Resize the ring buffer to a new capacity (drops oldest elements if needed)
        /// @param new_capacity The new capacity (must be greater than 0)
        /// @throws std::invalid_argument If new_capacity is 0
        void resize(size_type new_capacity)
        {
            if (new_capacity == capacity()) return;
            if (new_capacity == 0)
            {
                throw std::invalid_argument("RingBuffer capacity must be > 0");
            }

            std::vector<T> new_buffer(new_capacity + 1);
            const size_type copy_count = std::min(size(), new_capacity);
            for (size_type i = 0; i < copy_count; ++i)
            {
                new_buffer[i] = std::move(buffer_[(head_ + i) % capacity_]);
            }
            buffer_ = std::move(new_buffer);
            head_ = 0;
            tail_ = copy_count;
            capacity_ = new_capacity + 1;
        }

        /// @brief Swap the contents of this ring buffer with another
        /// @param other The ring buffer to swap with
        void swap(RingBuffer& other) noexcept
        {
            using std::swap;
            buffer_.swap(other.buffer_);
            swap(head_, other.head_);
            swap(tail_, other.tail_);
            swap(capacity_, other.capacity_);
        }

    private:
        std::vector<T> buffer_{};
        size_type head_ = 0;
        size_type tail_ = 0;
        size_type capacity_ = 0;

        /// @brief Advance to the next index (wrapping around)
        /// @param index Current index
        /// @return The next index
        [[nodiscard]] size_type next(size_type index) const noexcept
        {
            return (index + 1) % capacity_;
        }

        /// @brief Move to the previous index (wrapping around)
        /// @param index Current index
        /// @return The previous index
        [[nodiscard]] size_type prev(size_type index) const noexcept
        {
            return (index + capacity_ - 1) % capacity_;
        }
    };
}
