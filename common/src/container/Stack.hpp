#pragma once
#include <concepts>
#include <fmt/format.h>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace common::container {
/// @brief A simple stack implementation using a container.
/// @tparam T The type of elements stored in the stack.
/// @tparam Container The underlying container type used to store elements. Defaults to std::vector<T>.
template <std::movable T, typename Container = std::vector<T> >
class Stack {
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
    auto push(const T& value) -> void;

    /// @brief Pushes the given value onto the stack using move semantics.
    /// @param value The value to push.
    auto push(T&& value) -> void;

    /// @brief Constructs an element in-place on top of the stack.
    /// @tparam Args Types of arguments to forward to the constructor.
    /// @param args Arguments to forward to the constructor.
    template <typename... Args>
    auto emplace(Args&&... args) -> void;

    /// @brief Removes the top element from the stack.
    /// @throws std::out_of_range If the stack is empty.
    auto pop() -> void;

    /// @brief Accesses the top element of the stack.
    /// @return A reference to the top element.
    /// @throws std::out_of_range If the stack is empty.
    auto top() -> T&;

    /// @brief Accesses the top element of the stack.
    /// @return A const reference to the top element.
    /// @throws std::out_of_range If the stack is empty.
    auto top() const -> const T&;

    /// @brief Checks whether the stack is empty.
    /// @return True if the stack is empty, false otherwise.
    [[nodiscard]] auto empty() const noexcept -> bool;

    /// @brief Returns the number of elements in the stack.
    /// @return The number of elements in the stack.
    [[nodiscard]] auto size() const noexcept -> size_t;

    /// @brief Swaps the contents of this stack with another stack.
    /// @param other The stack to swap with.
    auto swap(Stack& other) noexcept -> void;

private:
    Container data_{};
};

template <std::movable T, typename Container>
Stack<T, Container>::Stack() = default;

template <std::movable T, typename Container>
template <std::input_iterator Iterator>
Stack<T, Container>::Stack(Iterator begin, Iterator end) : data_(begin, end) {
}

template <std::movable T, typename Container>
auto Stack<T, Container>::push(const T& value) -> void {
    data_.push_back(value);
}

template <std::movable T, typename Container>
auto Stack<T, Container>::push(T&& value) -> void {
    data_.push_back(std::move(value));
}

template <std::movable T, typename Container>
template <typename... Args>
auto Stack<T, Container>::emplace(Args&&... args) -> void {
    data_.emplace_back(std::forward<Args>(args)...);
}

template <std::movable T, typename Container>
auto Stack<T, Container>::pop() -> void {
    if (data_.empty()) {
        throw std::out_of_range("Stack is empty");
    }
    data_.pop_back();
}

template <std::movable T, typename Container>
auto Stack<T, Container>::top() -> T& {
    if (data_.empty()) {
        throw std::out_of_range("Stack is empty");
    }
    return data_.back();
}

template <std::movable T, typename Container>
auto Stack<T, Container>::top() const -> const T& {
    if (data_.empty()) {
        throw std::out_of_range("Stack is empty");
    }
    return data_.back();
}

template <std::movable T, typename Container>
auto Stack<T, Container>::empty() const noexcept -> bool {
    return data_.empty();
}

template <std::movable T, typename Container>
auto Stack<T, Container>::size() const noexcept -> size_t {
    return data_.size();
}

template <std::movable T, typename Container>
auto Stack<T, Container>::swap(Stack& other) noexcept -> void {
    using std::swap;
    swap(data_, other.data_);
}
}