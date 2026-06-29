/**
 * @file Stack.hpp
 * @brief Container-adapter stack with O(1) push/pop/top
 * @details A LIFO stack adapter built on a configurable underlying container
 *          (default std::vector<T>).  Provides O(1) amortised push, O(1) pop
 *          and top access.  Useful for depth-first traversal, expression
 *          evaluation, and undo/redo patterns.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 *
 * @par Usage Example
 * @code
 * Stack<int> s;
 * s.push(10);
 * s.push(20);
 * assert(s.top() == 20);
 * s.pop();
 * assert(s.top() == 10);
 * @endcode
 */

#pragma once
#include <concepts>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace cppforge::data_structure
{
    /// @brief A LIFO stack adapter over a configurable container.
    ///
    /// @tparam T         Element type (must satisfy std::movable).
    /// @tparam Container Underlying container type, defaults to std::vector<T>.
    ///
    /// @par Thread Safety
    /// This class is **not** thread-safe.  External synchronisation is required
    /// for concurrent access.
    ///
    /// @par Complexity
    /// - push / emplace:  O(1) amortised
    /// - pop / top:        O(1)
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
        /// @throws std::bad_alloc If memory allocation fails
        template <typename... Args>
        void emplace(Args&&... args);

        /// @brief Removes the top element from the stack.
        /// @throws std::out_of_range If the stack is empty.
        void pop();

        /// @brief Accesses the top element of the stack.
        /// @return A reference to the top element.
        /// @throws std::out_of_range If the stack is empty.
        [[nodiscard]] T& top();

        /// @brief Accesses the top element of the stack.
        /// @return A const reference to the top element.
        /// @throws std::out_of_range If the stack is empty.
        [[nodiscard]] const T& top() const;

        /// @brief Checks whether the stack is empty.
        /// @return True if the stack is empty, false otherwise.
        [[nodiscard]] bool empty() const noexcept;

        /// @brief Returns the number of elements in the stack.
        /// @return The number of elements in the stack.
        [[nodiscard]] std::size_t size() const noexcept;

        /// @brief Swaps the contents of this stack with another stack.
        /// @param other The stack to swap with.
        void swap(Stack& other) noexcept;

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
        data_.push_back(value);
    }

    template <std::movable T, typename Container>
    void Stack<T, Container>::push(T&& value)
    {
        data_.push_back(std::move(value));
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
    bool Stack<T, Container>::empty() const noexcept
    {
        return data_.empty();
    }

    template <std::movable T, typename Container>
    std::size_t Stack<T, Container>::size() const noexcept
    {
        return data_.size();
    }

    template <std::movable T, typename Container>
    void Stack<T, Container>::swap(Stack& other) noexcept
    {
        using std::swap;
        swap(data_, other.data_);
    }
}