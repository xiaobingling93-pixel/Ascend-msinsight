/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef PROFILER_SERVER_ERRDEF_H
#define PROFILER_SERVER_ERRDEF_H
#include <string>
namespace Dic {
class Status {
public:
    Status() = default;
    Status(bool s, std::string errMsg): isOk(s), errMsg(std::move(errMsg)) {};
    Status(bool s, std::string && errMsg): isOk(s), errMsg(std::move(errMsg)) {};
    explicit Status(bool s) : isOk(s) {};
    inline bool Ok() const { return isOk; }
    inline void SetOk() { isOk = true; }
    inline void SetErr() { isOk = false; }
    inline void SetErr(const std::string& msg)
    {
        isOk = false;
        errMsg = msg;
    }

    inline operator bool() const
    {
        return isOk;
    }

private:
    bool isOk{true};
    std::string errMsg;
};
}
#endif // PROFILER_SERVER_ERRDEF_H
