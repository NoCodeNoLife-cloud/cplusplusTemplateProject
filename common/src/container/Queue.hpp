/**
 * @file Queue.hpp
 * @brief Queue class declaration
 * @details This header defines the Queue class that provides functionality for Container data structures including Queue, Stack, Heap, and UnionSet.
 */

#pragma once
#include <concepts>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>
#include <utility>

namespace common::container
{
    /// @brief A queue data structure implementation using linked nodes with smart pointers
    /// @tparam T The type of elements stored in the queue
    template <std::movable T>
    class Queue
    {
    public:
        /// @brief Default constructor creates an empty queue
        Queue();

        /// @brief Copy constructor creates a deep copy of another queue
        /// @param other The queue to copy from
        Queue(const Queue& other);

        /// @brief Move constructor transfers ownership from another queue
        /// @param other The queue to move from
        Queue(Queue&& other) noexcept;

        /// @brief Copy assignment operator
        /// @param other The queue to copy from
        /// @return Reference to this queue
        Queue& operator=(const Queue& other);

        /// @brief Move assignment operator
        /// @param other The queue to move from
        /// @return Reference to this queue
        Queue& operator=(Queue&& other) noexcept;

        /// @brief Add an element to the back of the queue
        /// @param value The value to add
        /// @throws std::invalid_argument If the operation fails due to invalid state
        void push(const T& value);

        /// @brief Remove the element from the front of the queue
        /// @throws std::out_of_range If the queue is empty
        void pop();

        /// @brief Get a reference to the front element
        /// @return Reference to the front element
        /// @throws std::out_of_range If the queue is empty
        T& front();

        /// @brief Get a const reference to the front element
        /// @return Const reference to the front element
        /// @throws std::out_of_range If the queue is empty
        const T& front() const;

        /// @brief Get a reference to the back element
        /// @return Reference to the back element
        /// @throws std::out_of_range If the queue is empty
        T& back();

        /// @brief Get a const reference to the back element
        /// @return Const reference to the back element
        /// @throws std::out_of_range If the queue is empty
        const T& back() const;

        /// @brief Check if the queue is empty
        /// @return True if the queue is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept;

        /// @brief Get the number of elements in the queue
        /// @return The number of elements in the queue
        [[nodiscard]] size_t size() const noexcept;

        /// @brief Swap the contents of this queue with another
        /// @param other The queue to swap with
        void swap(Queue& other) noexcept;

    private:
        /// @brief Internal node structure for the queue
        struct Node
        {
            T data_{};
            std::unique_ptr<Node> next_{};

            /// @brief Constructor for a node
            /// @param value The value to store in the node
            explicit Node(T value);
        };

        std::unique_ptr<Node> head_{};
        Node* tail_{nullptr};
        size_t queue_size_{0};
    };

    template <std::movable T>
    Queue<T>::Queue() = default;

    template <std::movable T>
    Queue<T>::Queue(const Queue& other) : head_(nullptr), tail_(nullptr)
    {
        if (!other.empty())
        {
            head_ = std::make_unique<Node>(other.head_->data_);
            Node* current_new = head_.get();
            Node* current_other = other.head_.get();
            while (current_other->next_)
            {
                current_new->next_ = std::make_unique<Node>(current_other->next_->data_);
                current_new = current_new->next_.get();
                current_other = current_other->next_.get();
            }
            tail_ = current_new;
            queue_size_ = other.queue_size_;
        }
    }

    template <std::movable T>
    Queue<T>::Queue(Queue&& other) noexcept : head_(std::move(other.head_)), tail_(other.tail_), queue_size_(other.queue_size_)
    {
        other.tail_ = nullptr;
        other.queue_size_ = 0;
    }

    template <std::movable T>
    Queue<T>& Queue<T>::operator=(const Queue& other)
    {
        if (this != &other)
        {
            Queue copy(other);
            swap(copy);
        }
        return *this;
    }

    template <std::movable T>
    Queue<T>& Queue<T>::operator=(Queue&& other) noexcept
    {
        if (this != &other)
        {
            head_ = std::move(other.head_);
            tail_ = other.tail_;
            queue_size_ = other.queue_size_;
            other.tail_ = nullptr;
            other.queue_size_ = 0;
        }
        return *this;
    }

    template <std::movable T>
    void Queue<T>::push(const T& value)
    {
        auto new_node = std::make_unique<Node>(value);
        if (!new_node)
        {
            throw std::runtime_error("Queue::push: Failed to allocate memory for new node");
        }
        if (tail_)
        {
            tail_->next_ = std::move(new_node);
            tail_ = tail_->next_.get();
        }
        else
        {
            head_ = std::move(new_node);
            tail_ = head_.get();
        }
        ++queue_size_;
    }

    template <std::movable T>
    void Queue<T>::pop()
    {
        if (empty())
        {
            throw std::out_of_range("Queue is empty");
        }
        head_ = std::move(head_->next_);
        if (!head_)
        {
            tail_ = nullptr;
        }
        --queue_size_;
    }

    template <std::movable T>
    T& Queue<T>::front()
    {
        if (empty())
        {
            throw std::out_of_range("Queue is empty");
        }
        return head_->data_;
    }

    template <std::movable T>
    const T& Queue<T>::front() const
    {
        if (empty())
        {
            throw std::out_of_range("Queue is empty");
        }
        return head_->data_;
    }

    template <std::movable T>
    T& Queue<T>::back()
    {
        if (empty())
        {
            throw std::out_of_range("Queue is empty");
        }
        return tail_->data_;
    }

    template <std::movable T>
    const T& Queue<T>::back() const
    {
        if (empty())
        {
            throw std::out_of_range("Queue is empty");
        }
        return tail_->data_;
    }

    template <std::movable T>
    bool Queue<T>::empty() const noexcept
    {
        return queue_size_ == 0;
    }

    template <std::movable T>
    size_t Queue<T>::size() const noexcept
    {
        return queue_size_;
    }

    template <std::movable T>
    void Queue<T>::swap(Queue& other) noexcept
    {
        using std::swap;
        head_.swap(other.head_);
        swap(tail_, other.tail_);
        swap(queue_size_, other.queue_size_);
    }

    template <std::movable T>
    Queue<T>::Node::Node(T value) : data_(std::move(value)), next_(nullptr)
    {
    }
}