#pragma once
#include <glog/logging.h>
#include <fmt/format.h>
#include <memory>
#include <stdexcept>
#include <utility>

namespace common::container {
    /// @brief A queue data structure implementation using linked nodes with smart pointers
    /// @tparam T The type of elements stored in the queue
    template<typename T>
    class Queue {
    public:
        /// @brief Default constructor creates an empty queue
        Queue();

        /// @brief Copy constructor creates a deep copy of another queue
        /// @param other The queue to copy from
        Queue(const Queue &other);

        /// @brief Move constructor transfers ownership from another queue
        /// @param other The queue to move from
        Queue(Queue &&other) noexcept;

        /// @brief Copy assignment operator
        /// @param other The queue to copy from
        /// @return Reference to this queue
        auto operator=(const Queue &other) -> Queue &;

        /// @brief Move assignment operator
        /// @param other The queue to move from
        /// @return Reference to this queue
        auto operator=(Queue &&other) noexcept -> Queue &;

        /// @brief Add an element to the back of the queue
        /// @param value The value to add
        auto push(const T &value) -> void;

        /// @brief Remove the element from the front of the queue
        /// @throws std::out_of_range If the queue is empty
        auto pop() -> void;

        /// @brief Get a reference to the front element
        /// @return Reference to the front element
        /// @throws std::out_of_range If the queue is empty
        auto front() -> T &;

        /// @brief Get a const reference to the front element
        /// @return Const reference to the front element
        /// @throws std::out_of_range If the queue is empty
        auto front() const -> const T &;

        /// @brief Get a reference to the back element
        /// @return Reference to the back element
        /// @throws std::out_of_range If the queue is empty
        auto back() -> T &;

        /// @brief Get a const reference to the back element
        /// @return Const reference to the back element
        /// @throws std::out_of_range If the queue is empty
        auto back() const -> const T &;

        /// @brief Check if the queue is empty
        /// @return True if the queue is empty, false otherwise
        [[nodiscard]] auto empty() const noexcept -> bool;

        /// @brief Get the number of elements in the queue
        /// @return The number of elements in the queue
        [[nodiscard]] auto size() const noexcept -> size_t;

        /// @brief Swap the contents of this queue with another
        /// @param other The queue to swap with
        auto swap(Queue &other) noexcept -> void;

    private:
        /// @brief Internal node structure for the queue
        struct Node {
            T data_{};
            std::unique_ptr<Node> next_{};

            /// @brief Constructor for a node
            /// @param value The value to store in the node
            explicit Node(T value);
        };

        std::unique_ptr<Node> head_{};
        Node *tail_{nullptr};
        size_t queue_size_{0};
    };

    template<typename T>
    Queue<T>::Queue() = default;

    template<typename T>
    Queue<T>::Queue(const Queue &other) : head_(nullptr), tail_(nullptr) {
        if (!other.empty()) {
            head_ = std::make_unique<Node>(other.head_->data_);
            Node *current_new = head_.get();
            Node *current_other = other.head_.get();
            while (current_other->next_) {
                current_new->next_ = std::make_unique<Node>(current_other->next_->data_);
                current_new = current_new->next_.get();
                current_other = current_other->next_.get();
            }
            tail_ = current_new;
            queue_size_ = other.queue_size_;
        }
    }

    template<typename T>
    Queue<T>::Queue(Queue &&other) noexcept : head_(std::move(other.head_)), tail_(other.tail_), queue_size_(other.queue_size_) {
        other.tail_ = nullptr;
        other.queue_size_ = 0;
    }

    template<typename T>
    auto Queue<T>::operator=(const Queue &other) -> Queue & {
        if (this != &other) {
            Queue copy(other);
            swap(copy);
        }
        return *this;
    }

    template<typename T>
    auto Queue<T>::operator=(Queue &&other) noexcept -> Queue & {
        if (this != &other) {
            head_ = std::move(other.head_);
            tail_ = other.tail_;
            queue_size_ = other.queue_size_;
            other.tail_ = nullptr;
            other.queue_size_ = 0;
        }
        return *this;
    }

    template<typename T>
    auto Queue<T>::push(const T &value) -> void {
        auto new_node = std::make_unique<Node>(value);
        if (tail_) {
            tail_->next_ = std::move(new_node);
            tail_ = tail_->next_.get();
        } else {
            head_ = std::move(new_node);
            tail_ = head_.get();
        }
        ++queue_size_;
        DLOG(INFO) << fmt::format("Queue push - element added to back, size: {}", queue_size_);
    }

    template<typename T>
    auto Queue<T>::pop() -> void {
        if (empty()) {
            DLOG(ERROR) << "Queue pop failed - queue is empty";
            throw std::out_of_range("Queue is empty");
        }
        head_ = std::move(head_->next_);
        if (!head_) {
            tail_ = nullptr;
        }
        --queue_size_;
        DLOG(INFO) << fmt::format("Queue pop - front element removed, remaining size: {}", queue_size_);
    }

    template<typename T>
    auto Queue<T>::front() -> T & {
        if (empty()) {
            DLOG(ERROR) << "Queue front access failed - queue is empty";
            throw std::out_of_range("Queue is empty");
        }
        return head_->data_;
    }

    template<typename T>
    auto Queue<T>::front() const -> const T & {
        if (empty()) {
            DLOG(ERROR) << "Queue front access failed - queue is empty";
            throw std::out_of_range("Queue is empty");
        }
        return head_->data_;
    }

    template<typename T>
    auto Queue<T>::back() -> T & {
        if (empty()) {
            DLOG(ERROR) << "Queue back access failed - queue is empty";
            throw std::out_of_range("Queue is empty");
        }
        return tail_->data_;
    }

    template<typename T>
    auto Queue<T>::back() const -> const T & {
        if (empty()) {
            DLOG(ERROR) << "Queue back access failed - queue is empty";
            throw std::out_of_range("Queue is empty");
        }
        return tail_->data_;
    }

    template<typename T>
    auto Queue<T>::empty() const noexcept -> bool {
        return queue_size_ == 0;
    }

    template<typename T>
    auto Queue<T>::size() const noexcept -> size_t {
        return queue_size_;
    }

    template<typename T>
    auto Queue<T>::swap(Queue &other) noexcept -> void {
        using std::swap;
        head_.swap(other.head_);
        swap(tail_, other.tail_);
        swap(queue_size_, other.queue_size_);
    }

    template<typename T>
    Queue<T>::Node::Node(T value) : data_(std::move(value)), next_(nullptr) {
    }
}
