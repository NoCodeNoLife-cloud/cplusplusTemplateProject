/**
 * @file RbacManager.cc
 * @brief Role-Based Access Control manager implementation
 */

#include <cppforge/starter/security/RbacManager.hpp>
#include <algorithm>

namespace cppforge::starter::security
{
    void RbacManager::addRole(const Role& role)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        roles_[role.name] = role;
    }

    bool RbacManager::assignRole(const std::string& user_id, const std::string& role_name)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (roles_.find(role_name) == roles_.end())
        {
            return false;
        }

        user_roles_[user_id].insert(role_name);
        return true;
    }

    bool RbacManager::hasPermission(const std::string& user_id,
                                     const std::string& resource,
                                     const std::string& action) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto user_it = user_roles_.find(user_id);
        if (user_it == user_roles_.end())
        {
            return false;
        }

        for (const auto& role_name : user_it->second)
        {
            auto role_it = roles_.find(role_name);
            if (role_it == roles_.end())
            {
                continue;
            }

            for (const auto& perm : role_it->second.permissions)
            {
                if (perm.resource == resource && perm.action == action)
                {
                    return true;
                }
            }
        }

        return false;
    }

    bool RbacManager::revokeRole(const std::string& user_id, const std::string& role_name)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto user_it = user_roles_.find(user_id);
        if (user_it == user_roles_.end())
        {
            return false;
        }

        return user_it->second.erase(role_name) > 0;
    }

    std::vector<std::string> RbacManager::getUserRoles(const std::string& user_id) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = user_roles_.find(user_id);
        if (it == user_roles_.end())
        {
            return {};
        }

        return {it->second.begin(), it->second.end()};
    }

    bool RbacManager::hasRole(const std::string& role_name) const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return roles_.find(role_name) != roles_.end();
    }
}
