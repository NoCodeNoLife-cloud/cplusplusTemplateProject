#include "ConfigParam.hpp"

#include <filesystem>
#include <stdexcept>

namespace server_app::config
{
    ConfigParam& ConfigParam::getInstance()
    {
        static ConfigParam instance;
        return instance;
    }

    const std::string& ConfigParam::applicationDevConfigPath() const
    {
        if (!std::filesystem::exists(application_dev_config_path_))
        {
            throw std::runtime_error(
                "Application dev config path does not exist: " + application_dev_config_path_);
        }
        return application_dev_config_path_;
    }
}
