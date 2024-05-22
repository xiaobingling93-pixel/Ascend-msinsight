//
// Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
//

#ifndef PROFILER_SERVER_CMDUTIL_H
#define PROFILER_SERVER_CMDUTIL_H

#include <string>
#include "ServerLog.h"

namespace Dic {
class CmdUtil {
public:
    static inline bool ExecuteCmdWithResult(const std::string &cmd, std::string &result)
    {
        FILE *pipe = popen(cmd.c_str(), "r");
        if (!pipe) {
            Server::ServerLog::Info("popen cmd error, cmd: ", cmd);
            return false;
        }

        char buf[BUFFER_SIZE + 1] = {0};
        while (fgets(buf, BUFFER_SIZE, pipe) != NULL) {
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

private:
    static const int BUFFER_SIZE = 100;
};
}
#endif // PROFILER_SERVER_CMDUTIL_H
