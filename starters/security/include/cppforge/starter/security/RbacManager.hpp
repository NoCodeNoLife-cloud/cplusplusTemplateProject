/**
 * @file RbacManager.hpp
 * @brief Role-Based Access Control manager
 * @details Manages roles, permissions, and user-role assignments.
 *          Thread-safe for concurrent access.
 */

#pragma once
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <cppforge/starter/security/Permission.hpp>

namespace cppforge::starter::security
{
    /// @brief Role-Based Access Control manager
    /// @details Manages roles with permissions and assigns roles to users.
    ///          All operations are thread-safe.
    class RbacManager
    {
    public:
        RbacManager() = default;

        /// @brief Register a role with its permissions
        /// @param role The role to register
        void addRole(const Role& role);

        /// @brief Assign a role to a user
        /// @param user_id The user identifier
        /// @param role_name The name of the role to assign
        /// @return true if the role exists and was assigned
        bool assignRole(const std::string& user_id, const std::string& role_name);

        /// @brief Check if a user has a specific permission
        /// @param user_id The user identifier
        /// @param resource The resource to check
        /// @param action The action to check
        /// @return true if the user has the permission via any assigned role
        [[nodiscard]] bool hasPermission(const std::string& user_id,
                                         const std::string& resource,
                                         const std::string& action) const;

        /// @brief Revoke a role from a user
        /// @param user_id The user identifier
        /// @param role_name The name of the role to revoke
        /// @return true if the role was found and revoked
        bool revokeRole(const std::string& user_id, const std::string& role_name);

        /// @brief Get all roles assigned to a user
        /// @param user_id The user identifier
        /// @return Vector of role names assigned to the user
        [[nodiscard]] std::vector<std::string> getUserRoles(const std::string& user_id) const;

        /// @brief Check if a role is registered
        /// @param role_name The role name to check
        /// @return true if the role exists
        [[nodiscard]] bool hasRole(const std::string& role_name) const;

    private:
        mutable std::mutex mutex_;
        std::unordered_map<std::string, Role> roles_;
        std::unordered_map<std::string, std::unordered_set<std::string>> user_roles_;
    };
}
