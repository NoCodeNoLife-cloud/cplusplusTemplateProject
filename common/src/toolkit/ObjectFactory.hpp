/**
 * @file ObjectFactory.hpp
 * @brief Generic object factory for cloneable types
 * @details Implements the Factory Method pattern for creating copies or new
 *          instances of polymorphic types.  Maintains a registry of prototype
 *          instances keyed by type identifier (string or type_index).
 *          Supports cloning via prototype pattern.
 *
 * @par Thread Safety
 * This class is **not** thread-safe.  External synchronisation is required
 * for concurrent access.
 */

#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "interface/task/IStartupTask.hpp"

namespace cppforge::toolkit
{
    /// @brief A factory class for creating objects of type T.
    /// @tparam T The base type of objects that this factory can create.
    template <typename T>
    class ObjectFactory : public interface::task::IStartupTask
    {
    public:
        /// @brief Registers a type with the factory.
        /// @tparam V The type to register.
        /// @tparam Args The argument types for the constructor of V.
        /// @param type_name The name to register the type under.
        /// @param args The arguments to forward to the constructor of V.
        template <typename V, typename... Args>
        static void registerType(const std::string& type_name, Args&&... args);

        /// @brief Creates an object of the specified type.
        /// @param type_name The name of the type to create.
        /// @return A unique pointer to the created object.
        /// @throws std::runtime_error If the type name is not registered.
        [[nodiscard]] static std::unique_ptr<T> createObject(const std::string& type_name);

        /// @brief Checks if a type is registered in the factory.
        /// @param type_name The name of the type to check.
        /// @return true if the type is registered, false otherwise.
        [[nodiscard]] static bool isRegistered(const std::string& type_name);

        /// @brief Executes the startup task.
        /// @return True if successful, false otherwise.
        [[nodiscard]] bool execute()  override;

        /// @brief Clears all registered types from the factory.
        static void clearRegistry();

    private:
        /// @brief Registers all types with the factory.
        virtual void registerAll() = 0;

        /// @brief Get the registry map (using Meyer's singleton to ensure initialization order)
        static std::unordered_map<std::string, std::function < std::unique_ptr<T>()>
        >
        &
        getRegistry();

        /// @brief Get the mutex for the registry (using Meyer's singleton)
        static std::mutex& getRegistryMutex();
    };

    template <typename T>
    template <typename V, typename... Args>
    void ObjectFactory<T>::registerType(const std::string& type_name, Args&&... args)
    {
        if (type_name.empty())
        {
            throw std::invalid_argument("ObjectFactory::registerType: Type name cannot be empty");
        }

        std::lock_guard lock(getRegistryMutex());
        getRegistry()[type_name] = [args = std::make_tuple(std::forward<Args>(args)...)]() mutable
        {
            return std::apply([]<typename... T0>(T0&&... inner_args) -> std::unique_ptr<V>
            {
                return std::make_unique<V>(std::forward<T0>(inner_args)...);
            }, args);
        };
    }

    template <typename T>
    std::unique_ptr<T> ObjectFactory<T>::createObject(const std::string& type_name)
    {
        if (type_name.empty())
        {
            throw std::invalid_argument("ObjectFactory::createObject: Type name cannot be empty");
        }

        std::lock_guard lock(getRegistryMutex());
        auto it = getRegistry().find(type_name);
        if (it != getRegistry().end())
        {
            return it->second();
        }
        throw std::runtime_error("ObjectFactory::createObject: Unknown type: " + type_name);
    }

    template <typename T>
    bool ObjectFactory<T>::isRegistered(const std::string& type_name)
    {
        if (type_name.empty())
        {
            return false;
        }

        std::lock_guard lock(getRegistryMutex());
        return getRegistry().contains(type_name);
    }

    template <typename T>
    bool ObjectFactory<T>::execute()
    {
        try
        {
            registerAll();
            return true;
        }
        catch (...)
        {
            // In case registerAll throws, we catch the exception and return false
            return false;
        }
    }

    template <typename T>
    void ObjectFactory<T>::clearRegistry()
    {
        std::lock_guard lock(getRegistryMutex());
        getRegistry().clear();
    }

    template <typename T>
    std::unordered_map<std::string, std::function < std::unique_ptr<T>()>
    >
    &
    ObjectFactory<T>::getRegistry()
    {
        static std::unordered_map<std::string, std::function < std::unique_ptr<T>()>
        >
        registry{};
        return registry;
    }

    template <typename T>
    std::mutex& ObjectFactory<T>::getRegistryMutex()
    {
        static std::mutex registryMutex{};
        return registryMutex;
    }
}
