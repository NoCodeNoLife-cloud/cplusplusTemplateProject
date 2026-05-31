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

        /// @brief Get glog configuration file path
        /// @return Path string of the glog configuration file
        [[nodiscard]] const std::string& glogConfigPath() const;

        /// @brief Get application development environment configuration file path
        /// @return Path string of the application development environment configuration file
        [[nodiscard]] const std::string& applicationDevConfigPath() const;

    private:
        ConfigParam() = default;

        const std::string glog_config_path_{"../../client/src/config/glog-dev.yml"};
        std::string application_dev_config_path_{"../../client/src/config/application-dev.yml"};
    };
}
