/**
 * @file YamlToolkit.cc
 * @brief YamlToolkit class implementation
 * @details This file contains the implementation of the YamlToolkit class methods for Common library utilities.
 */

#include "filesystem/type/YamlToolkit.hpp"

#include <filesystem>
#include <fstream>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace common::filesystem
{
    bool YamlToolkit::create(const std::string& filepath, const YAML::Node& data)
    {
        try
        {
            if (const std::filesystem::path path(filepath); !path.parent_path().empty())
            {
                std::filesystem::create_directories(path.parent_path());
            }

            YAML::Emitter emitter;
            emitter << data;

            if (std::ofstream file(filepath); file.is_open())
            {
                file << emitter.c_str();
                return true;
            }
            return false;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    YAML::Node YamlToolkit::read(const std::string& filepath)
    {
        try
        {
            if (std::filesystem::exists(filepath))
            {
                return YAML::LoadFile(filepath);
            }
            return YAML::Node(YAML::NodeType::Undefined);
        }
        catch (const std::exception&)
        {
            return YAML::Node(YAML::NodeType::Undefined);
        }
    }

    bool YamlToolkit::update(const std::string& filepath, const YAML::Node& data)
    {
        return create(filepath, data);
    }

    bool YamlToolkit::remove(const std::string& filepath)
    {
        try
        {
            return std::filesystem::exists(filepath) && std::filesystem::remove(filepath);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    YAML::Node YamlToolkit::getValue(const std::string& filepath, const std::string& key)
    {
        try
        {
            if (const YAML::Node root = read(filepath); root.IsMap())
            {
                if (root[key].IsDefined())
                {
                    return root[key];
                }
            }
            return YAML::Node(YAML::NodeType::Undefined);
        }
        catch (const std::exception&)
        {
            return YAML::Node(YAML::NodeType::Undefined);
        }
    }

    bool YamlToolkit::setValue(const std::string& filepath, const std::string& key, const YAML::Node& value)
    {
        try
        {
            YAML::Node root = read(filepath);
            if (!root.IsMap())
            {
                root = YAML::Node(YAML::NodeType::Map);
            }

            root[key] = value;
            return create(filepath, root);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    namespace
    {
        void splitPath(const std::string& path, std::vector<std::string>& keys)
        {
            keys.clear();
            std::string::size_type prev = 0;
            std::string::size_type pos = 0;

            while ((pos = path.find('.', prev)) != std::string::npos)
            {
                if (pos > prev)
                {
                    keys.push_back(path.substr(prev, pos - prev));
                }
                prev = pos + 1;
            }

            if (prev < path.size())
            {
                keys.push_back(path.substr(prev));
            }
        }

        YAML::Node navigate(const YAML::Node& root, const std::vector<std::string>& keys)
        {
            YAML::Node current = root;
            for (const auto& key : keys)
            {
                if (!current.IsMap() || !current[key].IsDefined())
                {
                    return YAML::Node(YAML::NodeType::Undefined);
                }
                current = current[key];
            }
            return current;
        }
    }

    YAML::Node YamlToolkit::getNestedValue(const std::string& filepath, const std::string& path)
    {
        try
        {
            const YAML::Node root = read(filepath);
            if (path.empty())
            {
                return root;
            }
            if (!root.IsMap())
            {
                return {};
            }

            std::vector<std::string> keys;
            splitPath(path, keys);
            return navigate(root, keys);
        }
        catch (const std::exception&)
        {
            return {};
        }
    }

    bool YamlToolkit::setNestedValue(const std::string& filepath, const std::string& path, const YAML::Node& value)
    {
        try
        {
            YAML::Node root = read(filepath);
            if (!root.IsDefined() || !root.IsMap())
            {
                root = YAML::Node(YAML::NodeType::Map);
            }

            if (path.empty())
            {
                root = value;
            }
            else
            {
                std::vector<std::string> keys;
                splitPath(path, keys);

                YAML::Node current = root;
                for (std::size_t i = 0; i + 1 < keys.size(); ++i)
                {
                    if (!current[keys[i]].IsDefined())
                    {
                        current[keys[i]] = YAML::Node(YAML::NodeType::Map);
                    }
                    else if (!current[keys[i]].IsMap())
                    {
                        current[keys[i]] = YAML::Node(YAML::NodeType::Map);
                    }
                    current = current[keys[i]];
                }
                current[keys.back()] = value;
            }

            return create(filepath, root);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool YamlToolkit::hasKey(const std::string& filepath, const std::string& key)
    {
        try
        {
            if (const YAML::Node root = read(filepath); root.IsMap())
            {
                return root[key].IsDefined();
            }
            return false;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool YamlToolkit::merge(const std::string& filepath, const YAML::Node& data)
    {
        try
        {
            YAML::Node root = read(filepath);

            if (!data.IsMap())
            {
                return false;
            }

            if (!root.IsMap())
            {
                root = YAML::Node(YAML::NodeType::Map);
            }

            for (const auto& it : data)
            {
                root[it.first.as<std::string>()] = it.second;
            }

            return create(filepath, root);
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    std::string YamlToolkit::toString(const YAML::Node& node)
    {
        try
        {
            YAML::Emitter emitter;
            emitter << node;
            return emitter.c_str();
        }
        catch (const std::exception&)
        {
            return {};
        }
    }

    YAML::Node YamlToolkit::fromString(const std::string& str)
    {
        try
        {
            YAML::Node result = YAML::Load(str);
            if (!result.IsDefined())
            {
                return YAML::Node(YAML::NodeType::Undefined);
            }
            return result;
        }
        catch (const std::exception&)
        {
            return YAML::Node(YAML::NodeType::Undefined);
        }
    }

    YAML::Node YamlToolkit::getNodeOrRoot(const YAML::Node& root, const std::string& path)
    {
        try
        {
            if (path.empty())
            {
                return root;
            }

            std::vector<std::string> keys;
            splitPath(path, keys);
            YAML::Node current = root;

            for (const auto& key : keys)
            {
                if (!current.IsMap() || !current[key].IsDefined())
                {
                    return root;
                }
                current = current[key];
            }

            return current;
        }
        catch (const std::exception&)
        {
            return root;
        }
    }
}
