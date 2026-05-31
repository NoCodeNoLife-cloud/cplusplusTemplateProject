/**
 * @file Stack.hpp
 * @brief Stack class declaration
 * @details This header defines the Stack class that provides functionality for Container data structures including Queue, Stack, Heap, and UnionSet.
 */

#pragma once
#include <concepts>
#include <fmt/format.h>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace common::container
{
    /// @brief A simple stack implementation using a container.
    /// @tparam T The type of elements stored in the stack.
    /// @tparam Container The underlying container type used to store elements. Defaults to std::vector<T>.
    template <std::movable T, typename Container = std::vector<T>>
    class Stack
    {
    public:
        /// @brief Default constructor creates an empty stack
        Stack();

        /// @brief Constructor that initializes the stack with elements from an iterator range
        /// @tparam Iterator Type of the iterators
        /// @param begin Start iterator
        /// @param end End iterator
        template <std::input_iterator Iterator>
        Stack(Iterator begin, Iterator end);

        /// @brief Pushes a copy of the given value onto the stack.
        /// @param value The value to push.
        /// @throws std::bad_alloc If memory allocation fails
        void push(const T& value);

        /// @brief Pushes the given value onto the stack using move semantics.
        /// @param value The value to push.
        /// @throws std::bad_alloc If memory allocation fails
        void push(T&& value);

        /// @brief Constructs an element in-place on top of the stack.
        /// @tparam Args Types of arguments to forward to the constructor.
        /// @param args Arguments to forward to the constructor.
        template <typename... Args>
        void emplace(Args&&... args);

        /// @brief Removes the top element from the stack.
        /// @throws std::out_of_range If the stack is empty.
        void pop();

        /// @brief Accesses the top element of the stack.
        /// @return A reference to the top element.
        /// @throws std::out_of_range If the stack is empty.
        T& top();

        /// @brief Accesses the top element of the stack.
        /// @return A const reference to the top element.
        /// @throws std::out_of_range If the stack is empty.
        const T& top() const;

        /// @brief Checks whether the stack is empty.
        /// @return True if the stack is empty, false otherwise.
        [[nodiscard]] bool empty() const ;

        /// @brief Returns the number of elements in the stack.
        /// @return The number of elements in the stack.
        [[nodiscard]] std::size_t size() const ;

        /// @brief Swaps the contents of this stack with another stack.
        /// @param other The stack to swap with.
        void swap(Stack& other) ;

    private:
        Container data_{};
    };

    template <std::movable T, typename Container>
    Stack<T, Container>::Stack() = default;

    template <std::movable T, typename Container>
    template <std::input_iterator Iterator>
    Stack<T, Container>::Stack(Iterator begin, Iterator end) : data_(begin, end)
    {
    }

    template <std::movable T, typename Container>
    void Stack<T, Container>::push(const T& value)
    {
        try
        {
            data_.push_back(value);
        }
        catch (const std::bad_alloc&)
        {
            throw std::runtime_error("Stack::push: Failed to allocate memory for new element");
        }
    }

    template <std::movable T, typename Container>
    void Stack<T, Container>::push(T&& value)
    {
        try
        {
            data_.push_back(std::move(value));
        }
        catch (const std::bad_alloc&)
        {
            throw std::runtime_error("Stack::push: Failed to allocate memory for new element");
        }
    }

    template <std::movable T, typename Container>
    template <typename... Args>
    void Stack<T, Container>::emplace(Args&&... args)
    {
        data_.emplace_back(std::forward<Args>(args)...);
    }

    template <std::movable T, typename Container>
    void Stack<T, Container>::pop()
    {
        if (data_.empty())
        {
            throw std::out_of_range("Stack is empty");
        }
        data_.pop_back();
    }

    template <std::movable T, typename Container>
    T& Stack<T, Container>::top()
    {
        if (data_.empty())
        {
            throw std::out_of_range("Stack is empty");
        }
        return data_.back();
    }

    template <std::movable T, typename Container>
    const T& Stack<T, Container>::top() const
    {
        if (data_.empty())
        {
            throw std::out_of_range("Stack is empty");
        }
        return data_.back();
    }

    template <std::movable T, typename Container>
    bool Stack<T, Container>::empty() const
    {
        return data_.empty();
    }

    template <std::movable T, typename Container>
    std::size_t Stack<T, Container>::size() const
    {
        return data_.size();
    }

    template <std::movable T, typename Container>
    void Stack<T, Container>::swap(Stack& other)
    {
        using std::swap;
        swap(data_, other.data_);
    }
}