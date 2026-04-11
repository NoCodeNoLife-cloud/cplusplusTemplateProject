#pragma once
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "service/interface/IStartupTask.hpp"

namespace common::toolkit {
    /// @brief A factory class for creating objects of type T.
    /// @tparam T The base type of objects that this factory can create.
    template<typename T>
    class ObjectFactory : public service::interfaces::IStartupTask {
    public:
        /// @brief Registers a type with the factory.
        /// @tparam V The type to register.
        /// @tparam Args The argument types for the constructor of V.
        /// @param type_name The name to register the type under.
        /// @param args The arguments to forward to the constructor of V.
        template<typename V, typename... Args>
        static auto registerType(const std::string &type_name, Args &&... args) -> void;

        /// @brief Creates an object of the specified type.
        /// @param type_name The name of the type to create.
        /// @return A unique pointer to the created object.
        /// @throws std::runtime_error If the type name is not registered.
        [[nodiscard]] static auto createObject(const std::string &type_name) -> std::unique_ptr<T>;

        /// @brief Checks if a type is registered in the factory.
        /// @param type_name The name of the type to check.
        /// @return true if the type is registered, false otherwise.
        [[nodiscard]] static auto isRegistered(const std::string &type_name) -> bool;

        /// @brief Executes the startup task.
        /// @return True if successful, false otherwise.
        [[nodiscard]] auto execute() noexcept -> bool override;

        /// @brief Clears all registered types from the factory.
        static auto clearRegistry() -> void;

    private:
        /// @brief Registers all types with the factory.
        virtual auto registerAll() -> void = 0;

        /// @brief Get the registry map (using Meyer's singleton to ensure initialization order)
        static auto getRegistry() -> std::unordered_map<std::string, std::function<std::unique_ptr<T>()> > &;

        /// @brief Get the mutex for the registry (using Meyer's singleton)
        static auto getRegistryMutex() -> std::mutex &;
    };

    template<typename T>
    template<typename V, typename... Args>
    auto ObjectFactory<T>::registerType(const std::string &type_name, Args &&... args) -> void {
        if (type_name.empty()) {
            throw std::invalid_argument("ObjectFactory::registerType: Type name cannot be empty");
        }

        std::lock_guard<std::mutex> lock(getRegistryMutex());
        getRegistry()[type_name] = [args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
            return std::apply([]<typename... T0>(T0 &&... inner_args) -> std::unique_ptr<V> {
                return std::make_unique<V>(std::forward<T0>(inner_args)...);
            }, args);
        };
    }

    template<typename T>
    auto ObjectFactory<T>::createObject(const std::string &type_name) -> std::unique_ptr<T> {
        if (type_name.empty()) {
            throw std::invalid_argument("ObjectFactory::createObject: Type name cannot be empty");
        }

        std::lock_guard<std::mutex> lock(getRegistryMutex());
        auto it = getRegistry().find(type_name);
        if (it != getRegistry().end()) {
            return it->second();
        }
        throw std::runtime_error("ObjectFactory::createObject: Unknown type: " + type_name);
    }

    template<typename T>
    auto ObjectFactory<T>::isRegistered(const std::string &type_name) -> bool {
        if (type_name.empty()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(getRegistryMutex());
        return getRegistry().contains(type_name);
    }

    template<typename T>
    auto ObjectFactory<T>::execute() noexcept -> bool {
        try {
            registerAll();
            return true;
        } catch (...) {
            // In case registerAll throws, we catch the exception and return false
            return false;
        }
    }

    template<typename T>
    auto ObjectFactory<T>::clearRegistry() -> void {
        std::lock_guard<std::mutex> lock(getRegistryMutex());
        getRegistry().clear();
    }

    template<typename T>
    auto ObjectFactory<T>::getRegistry() -> std::unordered_map<std::string, std::function<std::unique_ptr<T>()> > & {
        static std::unordered_map<std::string, std::function<std::unique_ptr<T>()> > registry{};
        return registry;
    }

    template<typename T>
    auto ObjectFactory<T>::getRegistryMutex() -> std::mutex & {
        static std::mutex registryMutex{};
        return registryMutex;
    }
}
