#include "ConfigParam.h"

namespace client_app::config
{
    ConfigParam& ConfigParam::getInstance()
    {
        static ConfigParam instance;
        return instance;
    }

    const std::string& ConfigParam::glogConfigPath() const
    {
        return glog_config_path_;
    }

    const std::string& ConfigParam::applicationDevConfigPath() const
    {
        return application_dev_config_path_;
    }
}
