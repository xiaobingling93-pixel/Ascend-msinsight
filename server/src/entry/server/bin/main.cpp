// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.

#include "pch.h"
#include "ParamsParser.h"
#include "SocketUtil.h"
#include "WsServer.h"
#include "ProjectExplorerManager.h"
#include "JupyterServerManager.h"

using namespace std;
using namespace Dic;
using namespace Dic::Server;

namespace Dic {
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
    WsServer server(option.host, option.wsPort, option.sid);
    server.Start();
    const int checkInterval = 1000;
    while (server.IsStart()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(checkInterval));
    }
}

void PrintAvailablePort(int startPort)
{
    int port = startPort;
    const int scanRange = 100;
    while (port < startPort + scanRange) {
        if (SocketUtil::PortIsUsed(port)) {
            port++;
        } else {
            std::cout << "Available port: " << port << std::endl;
            return;
        }
    }
    std::cout << "[Error] Can't find port between " << startPort << " and " << port << std::endl;
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
    if (option.scanPort > 0) {
        PrintAvailablePort(option.scanPort);
        return 0;
    }
    ServerLog::Initialize(option.logPath, option.logSize, option.logLevel, to_string(option.wsPort));
    Dic::Module::Global::ProjectExplorerManager::Instance().InitSystemMemoryDbPath(option.logPath);
    ParamsOptionInfo();
    StartServer(option);
    return 0;
}