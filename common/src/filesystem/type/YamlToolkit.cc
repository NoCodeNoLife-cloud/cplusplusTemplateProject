#include "src/filesystem/type/YamlToolkit.hpp"

#include <glog/logging.h>
#include <fmt/format.h>
#include <yaml-cpp/yaml.h>
#include <string>
#include <fstream>
#include <filesystem>

namespace common::filesystem {
    auto YamlToolkit::create(const std::string &filepath, const YAML::Node &data) -> bool {
        try {
            DLOG(INFO) << fmt::format("YamlToolkit create - creating file: {}", filepath);
            // Create directory if it doesn't exist
            if (const std::filesystem::path path(filepath); !path.parent_path().empty()) {
                std::filesystem::create_directories(path.parent_path());
            }

            // Write data to file
            YAML::Emitter emitter;
            emitter << data;

            if (std::ofstream file(filepath); file.is_open()) {
                file << emitter.c_str();
                file.close();
                DLOG(INFO) << fmt::format("YamlToolkit create succeeded - file: {}", filepath);
                return true;
            }
            DLOG(ERROR) << fmt::format("YamlToolkit create failed - cannot open file for writing: {}", filepath);
            return false;
        } catch (const std::exception &e) {
            DLOG(ERROR) << fmt::format("YamlToolkit create failed - exception: {}, reason: {}", filepath, e.what());
            return false;
        }
    }

    auto YamlToolkit::read(const std::string &filepath) -> YAML::Node {
        try {
            if (std::filesystem::exists(filepath)) {
                DLOG(INFO) << fmt::format("YamlToolkit read - reading file: {}", filepath);
                return YAML::LoadFile(filepath);
            }
            DLOG(WARNING) << fmt::format("YamlToolkit read - file does not exist: {}", filepath);
            return {};
        } catch (const std::exception &e) {
            DLOG(ERROR) << fmt::format("YamlToolkit read failed - exception: {}, reason: {}", filepath, e.what());
            return {};
        }
    }

    auto YamlToolkit::update(const std::string &filepath, const YAML::Node &data) -> bool {
        try {
            // Update is essentially the same as create - overwrite the file with new data
            return create(filepath, data);
        } catch (const std::exception &) {
            return false;
        }
    }

    auto YamlToolkit::remove(const std::string &filepath) -> bool {
        try {
            if (std::filesystem::exists(filepath)) {
                DLOG(INFO) << fmt::format("YamlToolkit remove - removing file: {}", filepath);
                return std::filesystem::remove(filepath);
            }
            DLOG(WARNING) << fmt::format("YamlToolkit remove - file does not exist: {}", filepath);
            return false;
        } catch (const std::exception &e) {
            DLOG(ERROR) << fmt::format("YamlToolkit remove failed - exception: {}, reason: {}", filepath, e.what());
            return false;
        }
    }

    auto YamlToolkit::getValue(const std::string &filepath, const std::string &key) -> YAML::Node {
        try {
            if (const YAML::Node root = read(filepath); root.IsMap()) {
                return root[key];
            }
            return {};
        } catch (const std::exception &) {
            return {};
        }
    }

    auto YamlToolkit::setValue(const std::string &filepath, const std::string &key, const YAML::Node &value) -> bool {
        try {
            DLOG(INFO) << fmt::format("YamlToolkit setValue - setting key '{}' in file: {}", key, filepath);
            YAML::Node root = read(filepath);
            if (!root.IsMap()) {
                root = YAML::Node(YAML::NodeType::Map);
            }

            root[key] = value;
            return create(filepath, root);
        } catch (const std::exception &e) {
            DLOG(ERROR) << fmt::format("YamlToolkit setValue failed - exception: {}, reason: {}", key, e.what());
            return false;
        }
    }

    auto YamlToolkit::getNestedValue(const std::string &filepath, const std::string &path) -> YAML::Node {
        try {
            const YAML::Node root = read(filepath);
            if (!root.IsMap()) {
                return {};
            }

            YAML::Node current = root;
            std::string::size_type prev = 0;
            std::string::size_type pos = 0;

            while ((pos = path.find('.', prev)) != std::string::npos) {
                if (const std::string key = path.substr(prev, pos - prev); current[key]) {
                    current = current[key];
                    prev = pos + 1;
                } else {
                    return {};
                }
            }

            // Handle the last key
            const std::string key = path.substr(prev);
            return current[key];
        } catch (const std::exception &) {
            return {};
        }
    }

    auto YamlToolkit::setNestedValue(const std::string &filepath, const std::string &path, const YAML::Node &value) -> bool {
        try {
            DLOG(INFO) << fmt::format("YamlToolkit setNestedValue - setting path '{}' in file: {}", path, filepath);
            YAML::Node root = read(filepath);
            if (!root.IsMap()) {
                root = YAML::Node(YAML::NodeType::Map);
            }

            YAML::Node current = root;
            std::string::size_type prev = 0;
            std::string::size_type pos = 0;

            while ((pos = path.find('.', prev)) != std::string::npos) {
                const std::string key = path.substr(prev, pos - prev);
                if (!current[key]) {
                    current[key] = YAML::Node(YAML::NodeType::Map);
                }
                current = current[key];
                prev = pos + 1;
            }

            // Set the value at the final key
            const std::string key = path.substr(prev);
            current[key] = value;

            return create(filepath, root);
        } catch (const std::exception &e) {
            DLOG(ERROR) << fmt::format("YamlToolkit setNestedValue failed - exception: {}, reason: {}", path, e.what());
            return false;
        }
    }

    auto YamlToolkit::hasKey(const std::string &filepath, const std::string &key) -> bool {
        try {
            if (const YAML::Node root = read(filepath); root.IsMap()) {
                return root[key].IsDefined();
            }
            return false;
        } catch (const std::exception &) {
            return false;
        }
    }

    auto YamlToolkit::merge(const std::string &filepath, const YAML::Node &data) -> bool {
        try {
            DLOG(INFO) << fmt::format("YamlToolkit merge - merging data into file: {}", filepath);
            YAML::Node root = read(filepath);
            if (!root.IsMap() && !root.IsNull()) {
                DLOG(ERROR) << fmt::format("YamlToolkit merge failed - root is not a map or null node: {}", filepath);
                return false; // Can only merge with a map or null node
            }

            if (data.IsMap()) {
                if (!root.IsMap()) {
                    root = YAML::Node(YAML::NodeType::Map);
                }

                for (const auto &it: data) {
                    root[it.first.as<std::string>()] = it.second;
                }

                return create(filepath, root);
            }

            DLOG(ERROR) << fmt::format("YamlToolkit merge failed - data is not a map");
            return false;
        } catch (const std::exception &e) {
            DLOG(ERROR) << fmt::format("YamlToolkit merge failed - exception: {}, reason: {}", filepath, e.what());
            return false;
        }
    }

    auto YamlToolkit::toString(const YAML::Node &node) -> std::string {
        try {
            YAML::Emitter emitter;
            emitter << node;
            return {emitter.c_str()};
        } catch (const std::exception &) {
            return {};
        }
    }

    auto YamlToolkit::fromString(const std::string &str) -> YAML::Node {
        try {
            return YAML::Load(str);
        } catch (const std::exception &) {
            return {};
        }
    }

    auto YamlToolkit::getNodeOrRoot(const YAML::Node &root, const std::string &path) -> YAML::Node {
        try {
            if (root[path] && root[path].IsDefined()) {
                return root[path];
            }
            return root;
        } catch (const std::exception &) {
            return root;
        }
    }
}
