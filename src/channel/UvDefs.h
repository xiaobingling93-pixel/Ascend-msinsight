/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: UV Defines for DIC
 */
#ifndef DIC_UV_DEFS_H
#define DIC_UV_DEFS_H

#include <uv.h>

namespace Dic {
constexpr int UV_READABLE = 1;
constexpr int UV_BACKLOG = 128;
constexpr int UV_RET_OK = 0;
constexpr int UV_RET_FAIL = -1;
constexpr int UV_IPC_NO = 0;
constexpr int UV_IPC_YES = 1;

struct UvTty {
    uv_tty_t handle;
    void *arg = nullptr;
};

struct UvTcp {
    uv_tcp_t handle;
    void *arg = nullptr;
};

enum UvPipeType : int {
    T_STDIN = 0,
    T_STDOUT = 1,
    T_STDERR = 2
};

struct UvPipe {
    uv_pipe_t handle;
    void *arg = nullptr;
    UvPipeType type;
};

struct UvAsync {
    uv_async_t handle;
    void *arg = nullptr;
    int len;
};

struct UvReqConnect {
    uv_connect_t req;
    void *arg = nullptr;
};

enum class ChannelLinkMode {
    STD,
    SOCKET,
    PIPE,
    PROCESS,
};

enum class ChannelAccessType {
    READ,
    WRITE,
    BOTH
};

enum class ChannelServerType {
    SOCKET,
    PIPE
};
}

#endif // DIC_UV_DEFS_H
