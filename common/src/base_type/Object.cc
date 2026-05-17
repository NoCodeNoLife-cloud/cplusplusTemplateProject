/**
 * @file Object.cc
 * @brief Object class implementation
 * @details This file contains the implementation of the Object class methods for Base type classes providing fundamental object functionality.
 */

#include "src/base_type/Object.hpp"

#include <fmt/format.h>
#include <format>
#include <typeinfo>
#include <memory>
#include <string>

namespace common::base_type {
auto Object::getClass() const noexcept -> const std::type_info& {
    return typeid(*this);
}

auto Object::equals(const Object& other) const -> bool {
    return this == &other;
}

auto Object::hashCode() const noexcept -> size_t {
    return std::hash<const void*>{}(this);
}

auto Object::toString() const -> std::string {
    return std::string{getClass().name()} + "@" + std::to_string(hashCode());
}

auto Object::clone() const -> std::unique_ptr<Object> {
    // Since Object is likely to be used as a base for polymorphic types, 
    // we throw an exception to indicate that derived classes should implement this.
    throw std::logic_error("clone() must be implemented by derived classes");
}

auto Object::isInstance(const std::type_info& target_type) const noexcept -> bool {
    return target_type == typeid(*this);
}

auto Object::getClassName() const -> std::string {
    return std::string{getClass().name()};
}

auto Object::is(const Object& other) const noexcept -> bool {
    return this == &other;
}
}