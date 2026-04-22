#pragma once
#include <glog/logging.h>
#include <fmt/format.h>
#include <vector>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace common::container {
    /// @brief A simple stack implementation using a container.
    /// @tparam T The type of elements stored in the stack.
    /// @tparam Container The underlying container type used to store elements. Defaults to std::vector<T>.
    template<typename T, typename Container = std::vector<T> >
    class Stack {
    public:
        /// @brief Default constructor creates an empty stack
        Stack();

        /// @brief Constructor that initializes the stack with elements from an iterator range
        /// @tparam Iterator Type of the iterators
        /// @param begin Start iterator
        /// @param end End iterator
        template<typename Iterator>
        Stack(Iterator begin, Iterator end);

        /// @brief Pushes a copy of the given value onto the stack.
        /// @param value The value to push.
        auto push(const T &value) -> void;

        /// @brief Pushes the given value onto the stack using move semantics.
        /// @param value The value to push.
        auto push(T &&value) -> void;

        /// @brief Constructs an element in-place on top of the stack.
        /// @tparam Args Types of arguments to forward to the constructor.
        /// @param args Arguments to forward to the constructor.
        template<typename... Args>
        auto emplace(Args &&... args) -> void;

        /// @brief Removes the top element from the stack.
        /// @throws std::out_of_range If the stack is empty.
        auto pop() -> void;

        /// @brief Accesses the top element of the stack.
        /// @return A reference to the top element.
        /// @throws std::out_of_range If the stack is empty.
        auto top() -> T &;

        /// @brief Accesses the top element of the stack.
        /// @return A const reference to the top element.
        /// @throws std::out_of_range If the stack is empty.
        auto top() const -> const T &;

        /// @brief Checks whether the stack is empty.
        /// @return True if the stack is empty, false otherwise.
        [[nodiscard]] auto empty() const noexcept -> bool;

        /// @brief Returns the number of elements in the stack.
        /// @return The number of elements in the stack.
        [[nodiscard]] auto size() const noexcept -> size_t;

    private:
        Container data_{};
    };

    template<typename T, typename Container>
    Stack<T, Container>::Stack() = default;

    template<typename T, typename Container>
    template<typename Iterator>
    Stack<T, Container>::Stack(Iterator begin, Iterator end) : data_(begin, end) {
    }

    template<typename T, typename Container>
    auto Stack<T, Container>::push(const T &value) -> void {
        data_.push_back(value);
        DLOG(INFO) << fmt::format("Stack push - element added, size: {}", data_.size());
    }

    template<typename T, typename Container>
    auto Stack<T, Container>::push(T &&value) -> void {
        data_.push_back(std::move(value));
        DLOG(INFO) << fmt::format("Stack push (move) - element added, size: {}", data_.size());
    }

    template<typename T, typename Container>
    template<typename... Args>
    auto Stack<T, Container>::emplace(Args &&... args) -> void {
        data_.emplace_back(std::forward<Args>(args)...);
        DLOG(INFO) << fmt::format("Stack emplace - element constructed in-place, size: {}", data_.size());
    }

    template<typename T, typename Container>
    auto Stack<T, Container>::pop() -> void {
        if (data_.empty()) {
            DLOG(ERROR) << "Stack pop failed - stack is empty";
            throw std::out_of_range("Stack is empty");
        }
        data_.pop_back();
        DLOG(INFO) << fmt::format("Stack pop - top element removed, remaining size: {}", data_.size());
    }

    template<typename T, typename Container>
    auto Stack<T, Container>::top() -> T & {
        if (data_.empty()) {
            DLOG(ERROR) << "Stack top access failed - stack is empty";
            throw std::out_of_range("Stack is empty");
        }
        return data_.back();
    }

    template<typename T, typename Container>
    auto Stack<T, Container>::top() const -> const T & {
        if (data_.empty()) {
            DLOG(ERROR) << "Stack top access failed - stack is empty";
            throw std::out_of_range("Stack is empty");
        }
        return data_.back();
    }

    template<typename T, typename Container>
    auto Stack<T, Container>::empty() const noexcept -> bool {
        return data_.empty();
    }

    template<typename T, typename Container>
    auto Stack<T, Container>::size() const noexcept -> size_t {
        return data_.size();
    }
}
