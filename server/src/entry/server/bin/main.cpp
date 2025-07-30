// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#if defined(__linux__) || defined(__APPLE__)
#include <sys/resource.h>
#endif
#include "pch.h"
#include "ParamsParser.h"
#include "WsServer.h"

using namespace std;
using namespace Dic;
using namespace Dic::Server;

namespace Dic {
void SetResourceLimitForServer()
{
#if defined(__linux__) || defined(__APPLE__)
    struct rlimit oldResourceLimit;
    if (getrlimit(RLIMIT_NOFILE, &oldResourceLimit) == -1) {
        ServerLog::Error("Failed to execute getrlimit command.");
        return;
    }
    struct rlimit newResourceLimit;
    const int two = 2;
    newResourceLimit.rlim_cur = oldResourceLimit.rlim_max / two;
    newResourceLimit.rlim_max = oldResourceLimit.rlim_max;
    if (setrlimit(RLIMIT_NOFILE, &newResourceLimit) == -1) {
        const int defaultMaxFilesPerProc = 10240;
        newResourceLimit.rlim_cur = defaultMaxFilesPerProc;
        if (setrlimit(RLIMIT_NOFILE, &newResourceLimit) == -1) {
            ServerLog::Error("Failed to execute setrlimit command.");
            return;
        }
        ServerLog::Info("Set the soft limit of file descriptors to 10240 successfully.");
        return;
    }
    ServerLog::Info("Set the soft limit of file descriptors to the hard limit / 2 successfully.");
    return;
#endif
}

void ParamsOptionInfo()
{
    const ParamsOption &option = ParamsParser::Instance().GetOption();
    const int mbSize = 1024 * 1024;
    ServerLog::Info("Server Log Path: ", option.logPath);
    ServerLog::Info("Server Log Max Size: ", option.logSize / mbSize, "MB");
    ServerLog::Info("Server Log Level: ", option.logLevel);
}

void StartServer(const ParamsOption &option)
{
    ServerLog::Info("=============================== Server Start ===============================");
    WsServer server(option.host, option.wsPort);
    server.Start();
    const int checkInterval = 1000;
    while (server.IsStart()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(checkInterval));
    }
}
} // end of namespace Dic

int main(int argc, const char *argv[])
{
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.emplace_back(std::string(argv[i]));
    }
    const ParamsOption &option = ParamsParser::Instance().GetOption();
    if (!ParamsParser::Instance().Parse(args)) {
        ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, "");
        ServerLog::Error(ParamsParser::Instance().GetError());
        return -1;
    }

    ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
    ParamsOptionInfo();
    SetResourceLimitForServer();
    StartServer(option);
    return 0;
}