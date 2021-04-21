
#ifndef ARU_LOG_MODULE_NAME
#define ARU_LOG_MODULE_NAME "DEMO"
#endif

#include "aru/log/log.hpp"

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

#define DEMO_LOG(severity) ARU_LOG(severity, &__log)

static std::vector<aru::log::LogConfig> __config = {
    { ARU_LOG_MODULE_NAME, aru::log::LOG_INFO },
};

static aru::log::LogFilter __log(__config);

static DemoLog __demo_log;
