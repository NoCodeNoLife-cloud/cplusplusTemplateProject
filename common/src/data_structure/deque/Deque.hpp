/**
 * @file Deque.hpp
 * @brief Deque class declaration
 * @details This header defines the Deque class that provides a double-ended queue
 *          with O(1) amortized push/pop on both ends using a circular buffer.
 */

#pragma once
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace common::data_structure
{
    /// @brief A double-ended queue implementation using a circular buffer
    /// @tparam T The type of elements stored in the deque
    template <std::movable T>
    class Deque
    {
    public:
        /// @brief Default constructor creates an empty deque
        Deque() = default;

        /// @brief Copy constructor creates a deep copy of another deque
        /// @param other The deque to copy from
        Deque(const Deque& other);

        /// @brief Move constructor transfers ownership from another deque
        /// @param other The deque to move from
        Deque(Deque&& other) noexcept;

        /// @brief Copy assignment operator
        /// @param other The deque to copy from
        /// @return Reference to this deque
        Deque& operator=(const Deque& other);

        /// @brief Move assignment operator
        /// @param other The deque to move from
        /// @return Reference to this deque
        Deque& operator=(Deque&& other) noexcept;

        /// @brief Access the first element
        /// @return Reference to the first element
        /// @throws std::out_of_range If the deque is empty
        T& front();

        /// @brief Access the first element (const)
        /// @return Const reference to the first element
        /// @throws std::out_of_range If the deque is empty
        const T& front() const;

        /// @brief Access the last element
        /// @return Reference to the last element
        /// @throws std::out_of_range If the deque is empty
        T& back();

        /// @brief Access the last element (const)
        /// @return Const reference to the last element
        /// @throws std::out_of_range If the deque is empty
        const T& back() const;

        /// @brief Access element by index
        /// @param index Position of the element (0-based)
        /// @return Reference to the element at the given index
        /// @throws std::out_of_range If index is out of bounds
        T& at(size_t index);

        /// @brief Access element by index (const)
        /// @param index Position of the element (0-based)
        /// @return Const reference to the element at the given index
        /// @throws std::out_of_range If index is out of bounds
        const T& at(size_t index) const;

        /// @brief Access element by index without bounds checking
        /// @param index Position of the element (0-based)
        /// @return Reference to the element at the given index
        T& operator[](size_t index);

        /// @brief Access element by index without bounds checking (const)
        /// @param index Position of the element (0-based)
        /// @return Const reference to the element at the given index
        const T& operator[](size_t index) const;

        /// @brief Insert an element at the front
        /// @param value The value to insert
        void push_front(const T& value);

        /// @brief Insert an element at the front using move semantics
        /// @param value The value to move into the deque
        void push_front(T&& value);

        /// @brief Insert an element at the back
        /// @param value The value to insert
        void push_back(const T& value);

        /// @brief Insert an element at the back using move semantics
        /// @param value The value to move into the deque
        void push_back(T&& value);

        /// @brief Construct an element in-place at the front
        /// @tparam Args The types of the arguments to forward to T's constructor
        /// @param args The arguments to forward to T's constructor
        template <typename... Args>
        void emplace_front(Args&&... args);

        /// @brief Construct an element in-place at the back
        /// @tparam Args The types of the arguments to forward to T's constructor
        /// @param args The arguments to forward to T's constructor
        template <typename... Args>
        void emplace_back(Args&&... args);

        /// @brief Remove the first element
        /// @throws std::out_of_range If the deque is empty
        void pop_front();

        /// @brief Remove the last element
        /// @throws std::out_of_range If the deque is empty
        void pop_back();

        /// @brief Check if the deque is empty
        /// @return True if the deque is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept;

        /// @brief Get the number of elements in the deque
        /// @return The number of elements
        [[nodiscard]] size_t size() const noexcept;

        /// @brief Remove all elements from the deque
        void clear() noexcept;

        /// @brief Swap the contents of this deque with another
        /// @param other The deque to swap with
        void swap(Deque& other) noexcept;

    private:
        /// @brief The minimum capacity for the circular buffer
        static constexpr size_t kMinCapacity = 8;

        /// @brief Ensure capacity is available for new elements
        void reserve(size_t new_capacity);

        /// @brief Grow the circular buffer when full
        void grow();

        std::vector<T> buffer_{};
        size_t head_{0};
        size_t tail_{0};
        size_t deque_size_{0};
        size_t capacity_{0};
    };

    template <std::movable T>
    Deque<T>::Deque(const Deque& other)
    {
        if (other.deque_size_ > 0)
        {
            buffer_.resize(other.capacity_);
            for (size_t i = 0; i < other.deque_size_; ++i)
            {
                buffer_[i] = other.buffer_[(other.head_ + i) % other.capacity_];
            }
            head_ = 0;
            tail_ = other.deque_size_;
            deque_size_ = other.deque_size_;
            capacity_ = other.capacity_;
        }
    }

    template <std::movable T>
    Deque<T>::Deque(Deque&& other) noexcept
        : buffer_(std::move(other.buffer_))
        , head_(other.head_)
        , tail_(other.tail_)
        , deque_size_(other.deque_size_)
        , capacity_(other.capacity_)
    {
        other.head_ = 0;
        other.tail_ = 0;
        other.deque_size_ = 0;
        other.capacity_ = 0;
    }

    template <std::movable T>
    Deque<T>& Deque<T>::operator=(const Deque& other)
    {
        if (this != &other)
        {
            Deque copy(other);
            swap(copy);
        }
        return *this;
    }

    template <std::movable T>
    Deque<T>& Deque<T>::operator=(Deque&& other) noexcept
    {
        Deque(std::move(other)).swap(*this);
        return *this;
    }

    template <std::movable T>
    T& Deque<T>::front()
    {
        if (empty())
        {
            throw std::out_of_range("Deque is empty");
        }
        return buffer_[head_];
    }

    template <std::movable T>
    const T& Deque<T>::front() const
    {
        if (empty())
        {
            throw std::out_of_range("Deque is empty");
        }
        return buffer_[head_];
    }

    template <std::movable T>
    T& Deque<T>::back()
    {
        if (empty())
        {
            throw std::out_of_range("Deque is empty");
        }
        size_t back_index = (tail_ == 0) ? capacity_ - 1 : tail_ - 1;
        return buffer_[back_index];
    }

    template <std::movable T>
    const T& Deque<T>::back() const
    {
        if (empty())
        {
            throw std::out_of_range("Deque is empty");
        }
        size_t back_index = (tail_ == 0) ? capacity_ - 1 : tail_ - 1;
        return buffer_[back_index];
    }

    template <std::movable T>
    T& Deque<T>::at(size_t index)
    {
        if (index >= deque_size_)
        {
            throw std::out_of_range("Deque index out of range");
        }
        return buffer_[(head_ + index) % capacity_];
    }

    template <std::movable T>
    const T& Deque<T>::at(size_t index) const
    {
        if (index >= deque_size_)
        {
            throw std::out_of_range("Deque index out of range");
        }
        return buffer_[(head_ + index) % capacity_];
    }

    template <std::movable T>
    T& Deque<T>::operator[](size_t index)
    {
        return buffer_[(head_ + index) % capacity_];
    }

    template <std::movable T>
    const T& Deque<T>::operator[](size_t index) const
    {
        return buffer_[(head_ + index) % capacity_];
    }

    template <std::movable T>
    void Deque<T>::push_front(const T& value)
    {
        if (deque_size_ == capacity_)
        {
            grow();
        }
        head_ = (head_ == 0) ? capacity_ - 1 : head_ - 1;
        buffer_[head_] = value;
        ++deque_size_;
    }

    template <std::movable T>
    void Deque<T>::push_front(T&& value)
    {
        if (deque_size_ == capacity_)
        {
            grow();
        }
        head_ = (head_ == 0) ? capacity_ - 1 : head_ - 1;
        buffer_[head_] = std::move(value);
        ++deque_size_;
    }

    template <std::movable T>
    void Deque<T>::push_back(const T& value)
    {
        if (deque_size_ == capacity_)
        {
            grow();
        }
        buffer_[tail_] = value;
        tail_ = (tail_ + 1) % capacity_;
        ++deque_size_;
    }

    template <std::movable T>
    void Deque<T>::push_back(T&& value)
    {
        if (deque_size_ == capacity_)
        {
            grow();
        }
        buffer_[tail_] = std::move(value);
        tail_ = (tail_ + 1) % capacity_;
        ++deque_size_;
    }

    template <std::movable T>
    template <typename... Args>
    void Deque<T>::emplace_front(Args&&... args)
    {
        if (deque_size_ == capacity_)
        {
            grow();
        }
        head_ = (head_ == 0) ? capacity_ - 1 : head_ - 1;
        buffer_[head_] = T(std::forward<Args>(args)...);
        ++deque_size_;
    }

    template <std::movable T>
    template <typename... Args>
    void Deque<T>::emplace_back(Args&&... args)
    {
        if (deque_size_ == capacity_)
        {
            grow();
        }
        buffer_[tail_] = T(std::forward<Args>(args)...);
        tail_ = (tail_ + 1) % capacity_;
        ++deque_size_;
    }

    template <std::movable T>
    void Deque<T>::pop_front()
    {
        if (empty())
        {
            throw std::out_of_range("Deque is empty");
        }
        head_ = (head_ + 1) % capacity_;
        --deque_size_;
    }

    template <std::movable T>
    void Deque<T>::pop_back()
    {
        if (empty())
        {
            throw std::out_of_range("Deque is empty");
        }
        tail_ = (tail_ == 0) ? capacity_ - 1 : tail_ - 1;
        --deque_size_;
    }

    template <std::movable T>
    bool Deque<T>::empty() const noexcept
    {
        return deque_size_ == 0;
    }

    template <std::movable T>
    size_t Deque<T>::size() const noexcept
    {
        return deque_size_;
    }

    template <std::movable T>
    void Deque<T>::clear() noexcept
    {
        buffer_.clear();
        head_ = 0;
        tail_ = 0;
        deque_size_ = 0;
        capacity_ = 0;
    }

    template <std::movable T>
    void Deque<T>::swap(Deque& other) noexcept
    {
        using std::swap;
        buffer_.swap(other.buffer_);
        swap(head_, other.head_);
        swap(tail_, other.tail_);
        swap(deque_size_, other.deque_size_);
        swap(capacity_, other.capacity_);
    }

    template <std::movable T>
    void Deque<T>::reserve(size_t new_capacity)
    {
        if (new_capacity <= capacity_)
        {
            return;
        }

        std::vector<T> new_buffer(new_capacity);
        for (size_t i = 0; i < deque_size_; ++i)
        {
            new_buffer[i] = std::move(buffer_[(head_ + i) % capacity_]);
        }

        buffer_ = std::move(new_buffer);
        head_ = 0;
        tail_ = deque_size_;
        capacity_ = new_capacity;
    }

    template <std::movable T>
    void Deque<T>::grow()
    {
        size_t new_capacity = (capacity_ == 0) ? kMinCapacity : capacity_ * 2;
        reserve(new_capacity);
    }
}
