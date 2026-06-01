#pragma once
#include <string>

namespace client_app::config
{
    class ConfigParam
    {
    public:
        [[nodiscard]] static ConfigParam& getInstance();

        ConfigParam(const ConfigParam&) = delete;
        ConfigParam& operator=(const ConfigParam&) = delete;
        ConfigParam(ConfigParam&&) = delete;
        ConfigParam& operator=(ConfigParam&&) = delete;

        /// @brief Get application development environment configuration file path
        /// @return Path string of the application development environment configuration file
        [[nodiscard]] const std::string& applicationDevConfigPath() const;

    private:
        ConfigParam() = default;

        std::string application_dev_config_path_{"../../client/src/config/application-dev.yml"};
    };
}
