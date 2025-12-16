/*
 * -------------------------------------------------------------------------
 * This file is part of the MindStudio project.
 * Copyright (c) 2025 Huawei Technologies Co.,Ltd.
 *
 * MindStudio is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * -------------------------------------------------------------------------
 */

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
