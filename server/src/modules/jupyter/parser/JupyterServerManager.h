//
// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_JUPYTERSERVERMANAGER_H
#define PROFILER_SERVER_JUPYTERSERVERMANAGER_H

#include <memory>
#include <thread>

namespace Dic {
namespace Module {
namespace Jupyter {

struct JupyterServerInfo {
    std::string startDirectory;
    std::string url;
    std::string protocol;
    std::string host;
    std::string port;
    std::string path;
};

class JupyterServerManager {
public:
    static JupyterServerManager &Instance();
    std::string GetJupyterUrl(const std::string &filePath);
    bool Close();
    bool Reset(const std::string& path);
    bool Start(const std::string& path);
    bool IsJupyterRunning();
    bool ResetUnderRootPath(const std::string& path);
    void InitJupyterLogPath(const std::string &filePath);

private:
    JupyterServerManager();
    ~JupyterServerManager();
    bool InitJupyterServerInfo();
    void ClearJupyterServerInfo();
    void FillJupyterServerInfo(const std::string& url);
    static std::string GetPidByPort(std::string& port);
    static bool KillProcessByPid(std::string& pid);

    JupyterServerInfo jupyterServerInfo;
    std::shared_ptr<std::thread> jupyterThread;
    FILE *pipe;
    std::string jupyterLogPath;
    static const int maxRetryTimes = 5;
    const std::string jupyterUrlReg =
            R"(^(?:([a-z]+):\/\/)?([a-z0-9.-]+)(?::(\d+))?(\/[^?#]*)?$)";
    static const int urlProtocolPosition = 1;
    static const int urlHostPosition = 2;
    static const int urlPortPosition = 3;
    static const int urlPathPosition = 4;
};
}
}
}
#endif // PROFILER_SERVER_JUPYTERSERVERMANAGER_H
