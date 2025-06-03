//
// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_CMDUTIL_H
#define PROFILER_SERVER_CMDUTIL_H

#include <string>
#include "ServerLog.h"
#include "FileUtil.h"
#include "StringUtil.h"

namespace Dic {
class CmdUtil {
public:
    CmdUtil() = default;

    explicit CmdUtil(std::string execPath)
    {
        this->Command(std::move(execPath));
    }

    static inline bool ExecuteCmdWithResult(const std::string &cmd, std::string &result)
    {
        // 该公共方法，使用前建议进行命令注入问题检测
        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            Server::ServerLog::Error("popen cmd error, cmd: ", cmd);
            return false;
        }

        char buf[bufferSize + 1] = {0};
        while (fgets(buf, bufferSize, pipe) != nullptr) {
            result.append(buf);
        }

        // 获取状态，如果非0，则说明命令行执行有错误
        int status = pclose(pipe);
        if (status != 0) {
            return false;
        } else {
            return true;
        }
    }

    CmdUtil &Command(std::string execStr)
    {
        if (execStr.empty()) {
            badFlag = true;
            return *this;
        }
        if (!FileUtil::CheckDirValid(execStr) || !StringUtil::ValidateCommandFilePathParam(execStr)) {
            Server::ServerLog::Error("Cmd not valid");
            badFlag = true;
            return *this;
        }
        if (!FileUtil::CheckPathPermission(execStr, fs::perms::owner_exec)) {
            Server::ServerLog::Error("Cmd not have execute permission");
            badFlag = true;
            return *this;
        }
        this->exec = std::move(execStr);
        return *this;
    }

    CmdUtil &Args(std::string arg)
    {
        if (arg.empty()) {
            return *this;
        }
        // 检查args中是否包含非法字符
        if (!StringUtil::ValidateStringParam(arg)) {
            return *this;
        }
        args.emplace_back(std::move(arg));
        return *this;
    }

    bool ExecuteWithResult(std::string &result)
    {
        // 组装请求
        if (badFlag) {
            return false;
        }
        std::string command = exec + " " + StringUtil::join(args, " ");
        return ExecuteCmdWithResult(command, result);
    }

    inline bool Valid() { return !badFlag; }

private:
    static const int bufferSize = 100;
    std::string exec;
    std::vector<std::string> args;
    bool badFlag{false};
};
}
#endif // PROFILER_SERVER_CMDUTIL_H
