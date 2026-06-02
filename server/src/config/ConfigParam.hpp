#pragma once
#include <string>


namespace server_app::config
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
        /// @throws std::runtime_error If the configured path does not exist
        [[nodiscard]] const std::string& applicationDevConfigPath() const;

    private:
        ConfigParam() = default;

        const std::string application_dev_config_path_{"../../server/src/config/application-dev.yml"};
    };
}
