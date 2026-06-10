/**
 * @file LinkedList.hpp
 * @brief LinkedList class declaration
 * @details This header defines the LinkedList class — a doubly linked list with
 *          bidirectional iterators, providing O(1) insertion/removal at both ends
 *          and O(n) insertion/removal at arbitrary positions.
 */

#pragma once
#include <concepts>
#include <cstddef>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace common::data_structure
{
    /// @brief A doubly linked list implementation using std::unique_ptr for forward links
    /// @tparam T The type of elements stored in the list
    template <std::movable T>
    class LinkedList
    {
    public:
        using size_type = std::size_t;

    private:
        /// @brief Internal node structure for the linked list
        struct Node
        {
            T data_;
            std::unique_ptr<Node> next_;
            Node* prev_ = nullptr;

            template <typename... Args>
            explicit Node(Args&&... args)
                : data_(std::forward<Args>(args)...), next_(nullptr)
            {
            }
        };

    public:
        /// @brief Bidirectional iterator over list elements
        class iterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = T*;
            using reference = T&;

            iterator() = default;

            reference operator*() const { return node_->data_; }
            pointer operator->() const { return &node_->data_; }

            iterator& operator++()
            {
                node_ = node_->next_.get();
                return *this;
            }

            iterator operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            iterator& operator--()
            {
                node_ = node_->prev_;
                return *this;
            }

            iterator operator--(int)
            {
                auto tmp = *this;
                --(*this);
                return tmp;
            }

            bool operator==(const iterator& other) const { return node_ == other.node_; }
            bool operator!=(const iterator& other) const { return node_ != other.node_; }

        private:
            friend class LinkedList;
            explicit iterator(Node* node) : node_(node) {}
            Node* node_ = nullptr;
        };

        /// @brief Const bidirectional iterator over list elements
        class const_iterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;
            using pointer = const T*;
            using reference = const T&;

            const_iterator() = default;

            reference operator*() const { return node_->data_; }
            pointer operator->() const { return &node_->data_; }

            const_iterator& operator++()
            {
                node_ = node_->next_.get();
                return *this;
            }

            const_iterator operator++(int)
            {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            const_iterator& operator--()
            {
                node_ = node_->prev_;
                return *this;
            }

            const_iterator operator--(int)
            {
                auto tmp = *this;
                --(*this);
                return tmp;
            }

            bool operator==(const const_iterator& other) const { return node_ == other.node_; }
            bool operator!=(const const_iterator& other) const { return node_ != other.node_; }

        private:
            friend class LinkedList;
            explicit const_iterator(const Node* node) : node_(node) {}
            const Node* node_ = nullptr;
        };

        /// @brief Default constructor creates an empty list
        LinkedList() = default;

        /// @brief Copy constructor creates a deep copy of another list
        /// @param other The list to copy from
        LinkedList(const LinkedList& other)
        {
            if (other.empty()) return;
            head_ = std::make_unique<Node>(other.head_->data_);
            head_->prev_ = nullptr;
            Node* dst = head_.get();
            const Node* src = other.head_.get();
            while (src->next_)
            {
                src = src->next_.get();
                dst->next_ = std::make_unique<Node>(src->data_);
                dst->next_->prev_ = dst;
                dst = dst->next_.get();
            }
            tail_ = dst;
            list_size_ = other.list_size_;
        }

        /// @brief Move constructor transfers ownership from another list
        /// @param other The list to move from
        LinkedList(LinkedList&& other) noexcept
            : head_(std::move(other.head_))
            , tail_(other.tail_)
            , list_size_(other.list_size_)
        {
            other.tail_ = nullptr;
            other.list_size_ = 0;
        }

        /// @brief Copy assignment operator
        /// @param other The list to copy from
        /// @return Reference to this list
        LinkedList& operator=(const LinkedList& other)
        {
            if (this != &other)
            {
                LinkedList copy(other);
                swap(copy);
            }
            return *this;
        }

        /// @brief Move assignment operator
        /// @param other The list to move from
        /// @return Reference to this list
        LinkedList& operator=(LinkedList&& other) noexcept
        {
            LinkedList(std::move(other)).swap(*this);
            return *this;
        }

        /// @brief Destructor
        ~LinkedList() noexcept = default;

        // -- Element access --

        /// @brief Access the first element
        /// @return Reference to the first element
        /// @throws std::out_of_range If the list is empty
        [[nodiscard]] T& front()
        {
            if (empty()) throw std::out_of_range("LinkedList is empty");
            return head_->data_;
        }

        /// @brief Access the first element (const)
        /// @return Const reference to the first element
        /// @throws std::out_of_range If the list is empty
        [[nodiscard]] const T& front() const
        {
            if (empty()) throw std::out_of_range("LinkedList is empty");
            return head_->data_;
        }

        /// @brief Access the last element
        /// @return Reference to the last element
        /// @throws std::out_of_range If the list is empty
        [[nodiscard]] T& back()
        {
            if (empty()) throw std::out_of_range("LinkedList is empty");
            return tail_->data_;
        }

        /// @brief Access the last element (const)
        /// @return Const reference to the last element
        /// @throws std::out_of_range If the list is empty
        [[nodiscard]] const T& back() const
        {
            if (empty()) throw std::out_of_range("LinkedList is empty");
            return tail_->data_;
        }

        // -- Iterators --

        /// @brief Returns an iterator to the first element
        [[nodiscard]] iterator begin() { return iterator(head_.get()); }

        /// @brief Returns a const iterator to the first element
        [[nodiscard]] const_iterator begin() const { return const_iterator(head_.get()); }

        /// @brief Returns a const iterator to the first element
        [[nodiscard]] const_iterator cbegin() const { return begin(); }

        /// @brief Returns an iterator to the element following the last element
        [[nodiscard]] iterator end() { return iterator(nullptr); }

        /// @brief Returns a const iterator to the element following the last element
        [[nodiscard]] const_iterator end() const { return const_iterator(nullptr); }

        /// @brief Returns a const iterator to the element following the last element
        [[nodiscard]] const_iterator cend() const { return end(); }

        // -- Capacity --

        /// @brief Check if the list is empty
        /// @return True if the list is empty, false otherwise
        [[nodiscard]] bool empty() const noexcept { return list_size_ == 0; }

        /// @brief Get the number of elements in the list
        /// @return The number of elements
        [[nodiscard]] size_type size() const noexcept { return list_size_; }

        // -- Modifiers --

        /// @brief Insert an element at the front of the list
        /// @param value The value to insert
        void push_front(const T& value)
        {
            auto new_node = std::make_unique<Node>(value);
            new_node->next_ = std::move(head_);
            if (new_node->next_)
            {
                new_node->next_->prev_ = new_node.get();
            }
            else
            {
                tail_ = new_node.get();
            }
            head_ = std::move(new_node);
            ++list_size_;
        }

        /// @brief Insert an element at the front of the list using move semantics
        /// @param value The value to move into the list
        void push_front(T&& value)
        {
            auto new_node = std::make_unique<Node>(std::move(value));
            new_node->next_ = std::move(head_);
            if (new_node->next_)
            {
                new_node->next_->prev_ = new_node.get();
            }
            else
            {
                tail_ = new_node.get();
            }
            head_ = std::move(new_node);
            ++list_size_;
        }

        /// @brief Insert an element at the back of the list
        /// @param value The value to insert
        void push_back(const T& value)
        {
            auto new_node = std::make_unique<Node>(value);
            new_node->prev_ = tail_;
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
            ++list_size_;
        }

        /// @brief Insert an element at the back of the list using move semantics
        /// @param value The value to move into the list
        void push_back(T&& value)
        {
            auto new_node = std::make_unique<Node>(std::move(value));
            new_node->prev_ = tail_;
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
            ++list_size_;
        }

        /// @brief Construct an element in-place at the front of the list
        /// @tparam Args The types of the arguments to forward to T's constructor
        /// @param args The arguments to forward to T's constructor
        template <typename... Args>
        void emplace_front(Args&&... args)
        {
            auto new_node = std::make_unique<Node>(std::forward<Args>(args)...);
            new_node->next_ = std::move(head_);
            if (new_node->next_)
            {
                new_node->next_->prev_ = new_node.get();
            }
            else
            {
                tail_ = new_node.get();
            }
            head_ = std::move(new_node);
            ++list_size_;
        }

        /// @brief Construct an element in-place at the back of the list
        /// @tparam Args The types of the arguments to forward to T's constructor
        /// @param args The arguments to forward to T's constructor
        template <typename... Args>
        void emplace_back(Args&&... args)
        {
            auto new_node = std::make_unique<Node>(std::forward<Args>(args)...);
            new_node->prev_ = tail_;
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
            ++list_size_;
        }

        /// @brief Remove the first element from the list
        /// @throws std::out_of_range If the list is empty
        void pop_front()
        {
            if (empty()) throw std::out_of_range("LinkedList is empty");
            head_ = std::move(head_->next_);
            if (head_)
            {
                head_->prev_ = nullptr;
            }
            else
            {
                tail_ = nullptr;
            }
            --list_size_;
        }

        /// @brief Remove the last element from the list
        /// @throws std::out_of_range If the list is empty
        void pop_back()
        {
            if (empty()) throw std::out_of_range("LinkedList is empty");
            if (head_.get() == tail_)
            {
                head_.reset();
                tail_ = nullptr;
            }
            else
            {
                Node* new_tail = tail_->prev_;
                new_tail->next_.reset();
                tail_ = new_tail;
            }
            --list_size_;
        }

        /// @brief Insert an element before the given position
        /// @param pos Iterator before which to insert
        /// @param value The value to insert
        /// @return Iterator pointing to the inserted element
        iterator insert(iterator pos, const T& value)
        {
            if (pos == begin())
            {
                push_front(value);
                return begin();
            }
            if (pos == end())
            {
                push_back(value);
                return iterator(tail_);
            }
            auto new_node = std::make_unique<Node>(value);
            Node* next_node = pos.node_;
            Node* prev_node = next_node->prev_;

            new_node->prev_ = prev_node;
            new_node->next_ = std::move(prev_node->next_);
            prev_node->next_ = std::move(new_node);

            ++list_size_;
            return iterator(prev_node->next_.get());
        }

        /// @brief Insert an element before the given position using move semantics
        /// @param pos Iterator before which to insert
        /// @param value The value to move into the list
        /// @return Iterator pointing to the inserted element
        iterator insert(iterator pos, T&& value)
        {
            if (pos == begin())
            {
                push_front(std::move(value));
                return begin();
            }
            if (pos == end())
            {
                push_back(std::move(value));
                return iterator(tail_);
            }
            auto new_node = std::make_unique<Node>(std::move(value));
            Node* next_node = pos.node_;
            Node* prev_node = next_node->prev_;

            new_node->prev_ = prev_node;
            new_node->next_ = std::move(prev_node->next_);
            prev_node->next_ = std::move(new_node);

            ++list_size_;
            return iterator(prev_node->next_.get());
        }

        /// @brief Erase the element at the given position
        /// @param pos Iterator to the element to erase
        /// @return Iterator to the element following the erased one
        iterator erase(iterator pos)
        {
            if (pos == end()) return end();
            Node* target = pos.node_;
            Node* next_node = target->next_.get();
            Node* prev_node = target->prev_;
            if (prev_node)
            {
                prev_node->next_ = std::move(target->next_);
                if (prev_node->next_)
                {
                    prev_node->next_->prev_ = prev_node;
                }
                else
                {
                    tail_ = prev_node;
                }
            }
            else
            {
                head_ = std::move(target->next_);
                if (head_)
                {
                    head_->prev_ = nullptr;
                }
                else
                {
                    tail_ = nullptr;
                }
            }
            --list_size_;
            return iterator(next_node);
        }

        /// @brief Remove all elements with the given value
        /// @param value The value to remove
        /// @return Number of elements removed
        size_type remove(const T& value)
        {
            size_type count = 0;
            auto it = begin();
            while (it != end())
            {
                if (*it == value)
                {
                    it = erase(it);
                    ++count;
                }
                else
                {
                    ++it;
                }
            }
            return count;
        }

        /// @brief Remove all elements matching the predicate
        /// @tparam Predicate Unary predicate returning bool
        /// @param pred The predicate to match
        /// @return Number of elements removed
        template <typename Predicate>
        size_type remove_if(Predicate pred)
        {
            size_type count = 0;
            auto it = begin();
            while (it != end())
            {
                if (pred(*it))
                {
                    it = erase(it);
                    ++count;
                }
                else
                {
                    ++it;
                }
            }
            return count;
        }

        /// @brief Remove all elements from the list
        void clear() noexcept
        {
            head_.reset();
            tail_ = nullptr;
            list_size_ = 0;
        }

        /// @brief Reverse the order of elements in the list
        void reverse() noexcept
        {
            if (list_size_ < 2) return;

            tail_ = head_.get();
            std::unique_ptr<Node> prev;
            std::unique_ptr<Node> current = std::move(head_);

            while (current)
            {
                std::unique_ptr<Node> next = std::move(current->next_);
                current->next_ = std::move(prev);
                current->prev_ = next.get();
                prev = std::move(current);
                current = std::move(next);
            }

            head_ = std::move(prev);
            head_->prev_ = nullptr;
        }

        /// @brief Swap the contents of this list with another
        /// @param other The list to swap with
        void swap(LinkedList& other) noexcept
        {
            using std::swap;
            head_.swap(other.head_);
            swap(tail_, other.tail_);
            swap(list_size_, other.list_size_);
        }

        /// @brief Convert the list to a vector
        /// @return A vector containing all elements in order
        [[nodiscard]] std::vector<T> toVector() const
        {
            std::vector<T> result;
            result.reserve(list_size_);
            for (const auto& elem : *this)
            {
                result.push_back(elem);
            }
            return result;
        }

    private:
        std::unique_ptr<Node> head_{};
        Node* tail_ = nullptr;
        size_type list_size_ = 0;
    };
}
