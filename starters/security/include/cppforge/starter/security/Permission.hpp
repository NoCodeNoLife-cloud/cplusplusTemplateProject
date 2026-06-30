/**
 * @file Permission.hpp
 * @brief Permission and Role data structures for RBAC
 * @details Defines Permission (resource + action) and Role (name + permissions)
 *          used by the RbacManager for access control.
 */

#pragma once
#include <string>
#include <vector>

namespace cppforge::starter::security
{
    /// @brief Represents a single permission: an action on a resource
    struct Permission
    {
        /// @brief The resource being protected (e.g. "users", "orders")
        std::string resource;

        /// @brief The action allowed (e.g. "read", "write", "delete")
        std::string action;

        /// @brief Equality comparison
        bool operator==(const Permission& other) const
        {
            return resource == other.resource && action == other.action;
        }

        /// @brief Inequality comparison
        bool operator!=(const Permission& other) const
        {
            return !(*this == other);
        }
    };

    /// @brief Represents a named role with a set of permissions
    struct Role
    {
        /// @brief The role name (e.g. "admin", "editor", "viewer")
        std::string name;

        /// @brief The set of permissions granted by this role
        std::vector<Permission> permissions;
    };
}
