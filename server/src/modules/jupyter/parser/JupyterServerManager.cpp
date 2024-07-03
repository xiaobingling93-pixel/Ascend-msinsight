/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#include <cstdio>
#include "ServerLog.h"
#include "FileUtil.h"
#include "CmdUtil.h"
#include "JupyterServerManager.h"

namespace Dic {
namespace Module {
namespace Jupyter {
using namespace Server;
JupyterServerManager &JupyterServerManager::Instance()
{
    static JupyterServerManager instance;
    return instance;
}

bool JupyterServerManager::Close()
{
    // 根据port获取pid(外层已校验服务是否启动)
    std::string pid = GetPidByPort(jupyterServerInfo.port);
    if (pid.empty()) {
        ServerLog::Warn("Get pid by port failed!");
        return false;
    }

    // 根据pid杀死进程
    if (!KillProcessByPid(pid)) {
        ServerLog::Warn("Failed to kill process!");
        return false;
    }
    ClearJupyterServerInfo();
    return true;
}

std::string JupyterServerManager::GetPidByPort(std::string& port)
{
#ifdef WIN32
    std::string queryPidCmd = "for /f \"tokens=5 delims= \" %a in ('netstat -ano^|findstr 127.0.0.1:"
                              + port + "^| findstr LISTENING') do @echo %a";
#else
    std::string queryPidCmd = "lsof -i :" + port + " | grep -i LISTEN | awk '{print $2}'";
#endif
    std::string executeCmdResult;
    if (!CmdUtil::ExecuteCmdWithResult(queryPidCmd, executeCmdResult) || executeCmdResult.empty()) {
        ServerLog::Warn("Pid is not exist!");
        return "";
    }
    std::vector<std::string> pidList = StringUtil::Split(executeCmdResult, "[\r\n]");
    // 去除列表中空字符串
    pidList.erase(std::remove_if(pidList.begin(), pidList.end(),
        [](const std::string &str) {return str.empty();}), pidList.end());
    // 校验pid是否唯一
    std::string pid;
    for (const auto &item: pidList) {
        if (pid.empty()) {
            pid = item;
        } else if (pid != item) {
            ServerLog::Warn("Pid is not unique!");
            return "";
        }
    }
    return pid;
}

bool JupyterServerManager::KillProcessByPid(std::string& pid)
{
#ifdef WIN32
    std::string killProcessCmd = "taskkill /f /t /im " + pid;
#else
    std::string killProcessCmd = "kill " + pid;
#endif
    std::string cmdRes;
    if (!CmdUtil::ExecuteCmdWithResult(killProcessCmd, cmdRes)) {
        ServerLog::Error("Kill process by pid failed, pid:", pid);
        return false;
    }
    return true;
}

bool JupyterServerManager::Start(const std::string& path)
{
    ServerLog::Info("jupyter server start. start path:", path);
    // 如果jupyter文件存在，则清空jupyter日志，防止启动过程中，程序读取了老日志
    if (FileUtil::CheckDirAccess(jupyterLogPath, 0) && !FileUtil::RemoveFile(jupyterLogPath)) {
        ServerLog::Error("Failed to remove jupyter log file. ");
        return false;
    }
    // 获取路径的根目录，在根目录下启动
    std::shared_ptr<std::string> cmd = std::make_shared<std::string>(("jupyter-lab " + path +
                " --ServerApp.tornado_settings=\"{'headers': {'Content-Security-Policy': 'frame-ancestors "
                "\"self\" * wry://localhost'}}\" --ServerApp.disable_check_xsrf=True --ServerApp.token=\"\" "
                "--ServerApp.password=\"\" --no-browser --port 4000 "
                "--allow-root > " + jupyterLogPath + " 2>&1"));

    // 开启子进程启动jupyter服务
    pipe = popen(cmd->c_str(), "r");
    if (!pipe) {
        ServerLog::Warn("jupyter server start failed.");
        return false;
    }

    // 重试读取日志检查服务是否启动成功
    for (int i = 0; i < maxRetryTimes; ++i) {
        if (InitJupyterServerInfo()) {
            ServerLog::Info("jupyter server running!");
            jupyterServerInfo.startDirectory = path;
            return true;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    ServerLog::Warn("jupyter server failed to launch.");
    return false;
}

bool JupyterServerManager::IsJupyterRunning()
{
    if (jupyterServerInfo.port.empty()) {
        return false;
    }
    std::string cmd = "jupyter-lab list";
    std::string result;
    if (!CmdUtil::ExecuteCmdWithResult(cmd, result) || result.empty()) {
        // jupyter-lab查询失败，直接返回false
        ServerLog::Warn("jupyter-lab execute failed!");
        return false;
    }
    // 校验端口号是否存在，如果token存在则说明服务运行中
    if (result.find(jupyterServerInfo.port) != std::string::npos) {
        return true;
    } else {
        // 服务不存在，则清空缓存记录
        ClearJupyterServerInfo();
        return false;
    }
}

bool JupyterServerManager::ResetUnderRootPath(const std::string& path)
{
    std::string rootPath = FileUtil::GetRootPath(path);
    // 如果服务不在运行，则拉起jupyter服务
    if (!IsJupyterRunning()) {
        return Start(rootPath);
    }

    // 检查当前文件目录是否在启动目录下
    std::shared_ptr<std::string> relativePath = FileUtil::GetRelativePath(path, jupyterServerInfo.startDirectory);
    if (relativePath != nullptr) {
        // 如果结果不为空指针，说明当前文件在启动目录下，不需要重启，直接返回结果
        ServerLog::Info("jupyter server start directory is valid.");
        return true;
    }

    // 不在重启目录下，重启服务
    return Reset(rootPath);
}

bool JupyterServerManager::Reset(const std::string& path)
{
    ServerLog::Info("jupyter server restart.");
    // 关闭jupyter服务
    if (!Close()) {
        return false;
    }
    // 休息1秒，等待日志文件被正确释放（不休息可能出现日志文件被占用的问题）
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return Start(path);
}

std::string JupyterServerManager::GetJupyterUrl(const std::string &filePath)
{
    std::string res;
    // 检查服务信息是否已存在
    if (jupyterServerInfo.port.empty()) {
        return res;
    }
    // 初始化jupyter信息
    std::shared_ptr<std::string> relativePath =
            FileUtil::GetRelativePath(filePath, jupyterServerInfo.startDirectory);
    if (relativePath != nullptr) {
        res = jupyterServerInfo.protocol + "://" + jupyterServerInfo.host + ":" + jupyterServerInfo.port +
              "/lab/tree/" + *relativePath;
    }

    return res;
}

bool JupyterServerManager::InitJupyterServerInfo()
{
    if (!jupyterServerInfo.url.empty()) {
        return true;
    }
    std::string line;
    std::ifstream file(jupyterLogPath);
    std::string url;
    // 文件打开失败直接返回
    if (!file.is_open()) {
        return false;
    }
    while (getline(file, line)) {
        // 正则匹配获取localhost的链接
        std::regex url_regex("http://localhost:[^\\s]*");
        std::smatch match;
        // 找到后替换jupyter服务信息中的url
        if (std::regex_search(line, match, url_regex)) {
            url = match[0];
            break;
        }
    }
    file.close();

    if (!url.empty()) {
        FillJupyterServerInfo(url);
        return true;
    }
    return false;
}

void JupyterServerManager::ClearJupyterServerInfo()
{
    jupyterServerInfo.protocol = "";
    jupyterServerInfo.host = "";
    jupyterServerInfo.url = "";
    jupyterServerInfo.port = "";
    jupyterServerInfo.startDirectory = "";
    jupyterServerInfo.path = "";
}

void JupyterServerManager::FillJupyterServerInfo(const std::string& url)
{
    jupyterServerInfo.url = url;
    std::regex url_regex(jupyterUrlReg);
    std::smatch url_match;
    if (std::regex_match(url, url_match, url_regex)) {
        jupyterServerInfo.protocol = url_match[urlProtocolPosition].str();
        jupyterServerInfo.host = url_match[urlHostPosition].str();
        jupyterServerInfo.port = url_match[urlPortPosition].str();
        jupyterServerInfo.path = url_match[urlPathPosition].str();
    }
}

JupyterServerManager::JupyterServerManager() = default;

JupyterServerManager::~JupyterServerManager()
{
    Close();
}
}
}
}