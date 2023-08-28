/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: Entry of data insight core server binary
 */

#include <vector>
#include "TimeUtil.h"
#include "ServerLog.h"
#include "ParamsParser.h"
#include "WsServer.h"

using namespace std;
using namespace Dic;
using namespace Dic::Server;

namespace Dic {
void ParamsOptionInfo()
{
    const ParamsOption &option = ParamsParser::Instance().GetOption();
    const int mbSize = 1024 * 1024;
    ServerLog::Info("Websocket server port: ", option.wsPort);
    ServerLog::Info("Server Log Path: ", option.logPath);
    ServerLog::Info("Server Log Max Size: ", option.logSize / mbSize, "MB");
    ServerLog::Info("Server Log Level: ", option.logLevel);
}

void StartServer(const ParamsOption &option)
{
    ServerLog::Info("=============================== Data Insight Core Server Start ===============================");
    WsServer server(option.host, option.wsPort, option.sid);
    server.Start();
    while (server.IsStart()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    ServerLog::Info("=============================== Data Insight Core Server End =================================");
}
} // end of namespace Dic

int main(int argc, const char *argv[])
{
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++) {
        args.emplace_back(std::string(argv[i]));
    }
    if (!ParamsParser::Instance().Parse(args)) {
        ServerLog::Error(ParamsParser::Instance().GetError());
        return -1;
    }
    const ParamsOption &option = ParamsParser::Instance().GetOption();
    ServerLog::Initialize(option.logPath, option.logSize, option.logLevel);
    ParamsOptionInfo();
    StartServer(option);
    return 0;
}