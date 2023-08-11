/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Description: Protocol Enums declaration
 */

#ifndef DIC_PROTOCOL_ENUM_H
#define DIC_PROTOCOL_ENUM_H

namespace Dic {
namespace Protocol {
enum class SceneType : int {
    UNKNOWN = -1,
    GLOBAL = 1,
    DATABASE = 2,
    TOOL = 4,
    LOG = 8,
    HARMONY = 256,
};

enum class LinkType : int {
    SOCKET = 0,
    WEBSOCKET
};

// harmony
enum class DeviceStatus : int {
    OFFLINE = 0,
    ONLINE
};

enum class ProcessStatus : int {
    ALIVE = 0,
    DEAD
};

enum class DeviceConnectType : int {
    USB = 0,
    TCP
};

enum class ApplicationStatus : int {
    UN_INSTALLED = 0,
    INSTALLED,
    RUNNING
};

enum class ProcessType : int {
    SYSTEM_PROCESS = 0,
    MAIN_PROCESS,
    EXTENSION_PROCESS,
    RENDER_PROCESS
};
} // end of namespace Protocol
} // end if namespace Dic

#endif // DIC_PROTOCOL_ENUM_H
