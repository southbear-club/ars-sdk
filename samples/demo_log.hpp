
#ifndef ARS_LOG_MODULE_NAME
#define ARS_LOG_MODULE_NAME "DEMO"
#endif

#include "ars/sdk/log/log.hpp"

class DemoLog {
public:
    DemoLog() {
        google::InitGoogleLogging(__FILE__);
        // 输出到标准输出而不是文件
        FLAGS_logtostderr = true;
        // 除了输出到文件，也输出到终端
        FLAGS_alsologtostderr = true;

        // 打印色彩
        FLAGS_colorlogtostderr = true;
        FLAGS_minloglevel = google::GLOG_INFO;
    }
};

#define DEMO_LOG(severity) ARS_LOG(severity, &__log)

static std::vector<ars::log::LogConfig> __config = {
    { ARS_LOG_MODULE_NAME, ars::log::LOG_INFO },
};

static ars::log::LogFilter __log(__config);

static DemoLog __demo_log;
