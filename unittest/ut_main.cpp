/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file ut_main.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-03
 * 
 * @copyright MIT
 * 
 */
#include <stdio.h>
#include <stdlib.h>

#include <glog/logging.h>
#include <glog/raw_logging.h>
#include "gtest/gtest.h"

#include "ut_log.hpp"

using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;

namespace ttt
{

typedef struct {
    size_t a;
} st1_t;

class LogOutTest {
public:
    LogOutTest() {}

    void get_st1(st1_t &t) {
        t.a = 10;
        LOG(INFO) << "t.a = " << t.a << "\n";
    }
};

} // namespace ttt

class AruEnvironment : public testing::Environment {
public:
    virtual void SetUp() {
        ttt::LogOutTest ts;
        ttt::st1_t t1;
        ts.get_st1(t1);
        std::cout << "AruEnvironment SetUp" << std::endl;

        // 输出到标准输出而不是文件
        FLAGS_logtostderr = true;
        // 除了输出到文件，也输出到终端
        FLAGS_alsologtostderr = true;

        // 打印色彩
        FLAGS_colorlogtostderr = true;
        FLAGS_minloglevel = google::GLOG_INFO;

        std::vector<ars::log::LogConfig> logs = {
            {"main", ars::log::LOG_INFO},
            {"log", ars::log::LOG_INFO},
            {"memory", ars::log::LOG_INFO},
        };

        get_log_filter()->setup_log(logs);
    }
    virtual void TearDown() { std::cout << "AruEnvironment TearDown" << std::endl; }
};

int main(int argc, char** argv) {
#ifdef DEBUG
    printf("-------------------------ars unittest-----------------------\n");
#endif

    testing::AddGlobalTestEnvironment(new AruEnvironment);
    InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}